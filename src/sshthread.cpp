/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#endif

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifndef INADDR_NONE
#define INADDR_NONE (in_addr_t)-1
#endif

#include <libssh2.h>

#include <QThread>
#include <QDebug>
#include <QException>
#include <QFileInfo>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>

#include "sshdbconnection.h"
#include "sshthread.h"

enum {
    AUTH_NONE = 0,
    AUTH_PASSWORD,
    AUTH_PUBLICKEY
};

int SshThread::nLibSshUsers = 0;

// this variable is the local port number to bind to the forwarding connection.
// the default is set to the typical upper bound of the ephemeral port range
// to minimise conflicts. It will be incremented for each invocation of the class
// (so you might run into problems if you create > 3535 connections without restarting)
int SshThread::localBoundPort = 62000;

SshThread::SshThread(SshParameters& params) :
    params_(params)
{
    sock_ = -1;
    sock_listen_ = -1;
    sock_fwd_ = -1;
    if(nLibSshUsers == 0) {
#ifdef WIN32
        WSADATA wsadata;
        WSAStartup(MAKEWORD(2,0), &wsadata);
#endif
        if(libssh2_init(0) != 0) {
            fprintf(stderr, "libssh2 initialization failed");
            qApp->exit(1);
        }
        nLibSshUsers++;
    }
}

SshThread::~SshThread()
{
    if(--nLibSshUsers == 0)
        libssh2_exit();
}


bool SshThread::createSocket() {
    struct addrinfo* res;

    { // get address info
        struct addrinfo hints = {0};
        hints.ai_family = PF_INET; // | PF_INET6
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        int rc = getaddrinfo(params_.sshHost_.constData(), params_.sshPort_.constData(), &hints, &res);
        if(rc != 0) {
            emit tunnelFailed(rc == EAI_SYSTEM ? strerror(errno) : gai_strerror(rc));
            return false;
        }
    }

    int lastError = 0;
    for(struct addrinfo* p = res; p; p = p->ai_next) {
        if((sock_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            lastError = errno;
            continue;
        }

        if(::connect(sock_, p->ai_addr, p->ai_addrlen) != 0) {
            lastError = errno;
            close(sock_);
            sock_ = -1;
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if(lastError || sock_ < 0) {
        emit tunnelFailed(strerror(lastError));
        return false;
    }

    return true;
}

bool SshThread::createSession() {
    session = libssh2_session_init();
    if(session == nullptr) {
        emit tunnelFailed("libssh2_session_init failed");
        return false;
    }

    // trade banners, keys, setup crypto, compression, MAC
    int rc = libssh2_session_handshake(session, sock_);
    if(rc) {
        char* msg = 0;
        libssh2_session_last_error(session, &msg, nullptr, 0);
        emit tunnelFailed(msg);
        free(msg);
        return false;
    }

    LIBSSH2_KNOWNHOSTS* knownHosts = libssh2_knownhost_init(session);
    if(knownHosts == nullptr)
        return false;
    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    if(!dataDir.exists())
        dataDir.mkpath(dataDir.path());
    QByteArray knownHostsFile = dataDir.filePath("known_hosts").toLocal8Bit();
    qDebug() << libssh2_knownhost_readfile(knownHosts, knownHostsFile.constData(), LIBSSH2_KNOWNHOST_FILE_OPENSSH);
    int type;
    size_t len;
    const char* fingerprint = libssh2_session_hostkey(session, &len, &type);

    bool fingerprintOk = false;

    if(fingerprint) {
        struct libssh2_knownhost *host;
        qDebug() << params_.sshHost_.constData();
        qDebug() << QString(params_.sshPort_).toInt();
        int check = libssh2_knownhost_checkp(knownHosts, params_.sshHost_.constData(), QString(params_.sshPort_).toInt(),
                        fingerprint, len, LIBSSH2_KNOWNHOST_TYPE_PLAIN|LIBSSH2_KNOWNHOST_KEYENC_RAW, &host);

        fprintf(stderr, "Host check: %d, key: %s\n", check,
                (check <= LIBSSH2_KNOWNHOST_CHECK_MISMATCH)?
                host->key:"<none>");

        char readableFingerprint[41];
        for(int i = 0; i < 20; i++)
            sprintf(&readableFingerprint[i<<1], "%02X", fingerprint[i]);

        switch(check) {
        case LIBSSH2_KNOWNHOST_CHECK_MATCH:
            fingerprintOk = true;
            break;
        case LIBSSH2_KNOWNHOST_CHECK_MISMATCH:
            emit tunnelFailed("Server host key does not match that in known_hosts file");
            break;
        case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND:
            emit confirmUnknownHost(readableFingerprint, &fingerprintOk);
            if(fingerprintOk) {
                libssh2_knownhost_addc(knownHosts, params_.sshHost_.constData(), params_.sshHost_.constData(), fingerprint, len,
                    nullptr, 0, LIBSSH2_KNOWNHOST_TYPE_PLAIN|LIBSSH2_KNOWNHOST_KEYENC_RAW|LIBSSH2_KNOWNHOST_KEY_SSHRSA, nullptr);
                libssh2_knownhost_writefile(knownHosts, knownHostsFile.constData(), LIBSSH2_KNOWNHOST_FILE_OPENSSH);
            } else
                emit tunnelFailed(QString{});
            break;
        case LIBSSH2_KNOWNHOST_CHECK_FAILURE:
            emit tunnelFailed("Failed to check known hosts");
            break;
        default:
            Q_ASSERT("Branch can't happen" == 0);
            break;
        }
    } else {
        emit tunnelFailed("Failed to get server host key");
    }
    libssh2_knownhost_free(knownHosts);

    if(!fingerprintOk)
        return false;

    return true;
}


bool SshThread::authenticate() {

    const char* userauthlist = libssh2_userauth_list(session, params_.sshUser_.constData(), params_.sshUser_.length());

    if(params_.useSshKey_) {
        if(strstr(userauthlist, "publickey") == nullptr) {
            emit tunnelFailed("Public Key authentication method not supported by server");
            return false;
        }

        // libssh2 needs the corresponding public key in order to authenticate. If compiled against
        // openssl, it can generate it itself (pass nullptr to libssh2_userauth_publickey_fromfile_ex)
        // from the private key. If compiled against gnutls, this will fail. Until this is resolved,
        // first attempt to load a public key from <path/to/privatekey>.pub - if this file doesn't
        // exist, cross fingers, pass nullptr, and hope we're running against openssl :)

        QByteArray publicKeyPath = params_.sshKeyPath_ + ".pub";
        bool useGuessedPublicKey = false;
        { // guess the path to public key
            QFileInfo guessedPublicKey(publicKeyPath);
            if(guessedPublicKey.exists() && guessedPublicKey.isReadable())
                useGuessedPublicKey = true;
        }

        if(libssh2_userauth_publickey_fromfile_ex(session,
                params_.sshUser_.constData(), params_.sshUser_.length(),
                       useGuessedPublicKey ? publicKeyPath.constData() : nullptr, params_.sshKeyPath_.constData(), nullptr))
            return shutdown("Authentication by public key failed!\n");
        fprintf(stderr, "Authentication by public key succeeded.\n");
    } else {

        if(strstr(userauthlist, "password") == nullptr) {
            emit tunnelFailed("Password authentication method not supported by server");
            return false;
        }

        if (libssh2_userauth_password_ex(session,
                params_.sshUser_.constData(), params_.sshUser_.length(),
                params_.sshPass_.constData(), params_.sshPass_.length(), nullptr))
            return shutdown("Authentication by password failed.\n");
    }

    return true;

}

bool SshThread::setupTunnel() {
#ifdef WIN32
    char sockopt;
#else
    int sockopt;
#endif

    struct sockaddr_in sin;

    int localListenPort = localBoundPort++;
    sock_listen_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(localListenPort);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(sin.sin_addr.s_addr == INADDR_NONE) {
        emit tunnelFailed(strerror(errno));
        return false;
    }

    sockopt = 1;
    setsockopt(sock_listen_, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));

    socklen_t sinlen = sizeof(sin);
    qDebug() << "attempting to bind to local port " << localListenPort;

    if(bind(sock_listen_, (struct sockaddr *)&sin, sinlen) < 0) {
        emit tunnelFailed(strerror(errno));
        return false;
    }

    if(listen(sock_listen_, 2) < 0) {
        emit tunnelFailed(strerror(errno));
        return false;
    }

    qDebug() << "Waiting for TCP connection on" << inet_ntoa(sin.sin_addr) << "port" << ntohs(sin.sin_port);

    // actually we shouldn't emit until after accept, but that's tricky
    emit sshTunnelOpened(localListenPort);

    sock_fwd_ = accept(sock_listen_, (struct sockaddr *)&sin, &sinlen);
    if(sock_fwd_ < 0) {
        emit tunnelFailed(strerror(errno));
        return false;
    }

    const char* shost = inet_ntoa(sin.sin_addr);
    unsigned int sport = ntohs(sin.sin_port);

    qDebug() << "Forwarding connection from" << shost << ":" << sport << "to remote" << params_.remoteHost().constData() << ":" << params_.remotePort();

    channel = libssh2_channel_direct_tcpip_ex(session, params_.remoteHost().constData(), params_.remotePort(), shost, sport);
    if (!channel) {
        emit tunnelFailed("Could not open direct TCP/IP channel. Please review the server logs");
        return false;
    }

    return true;
}

void SshThread::routeTraffic() {
    libssh2_session_set_blocking(session, 0);

    // select loop
    struct timeval tv;
    fd_set fds;
    char buf[16384];

    while(true) {
        FD_ZERO(&fds);
        FD_SET(sock_fwd_, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;

        int rc = select(sock_fwd_ + 1, &fds, nullptr, nullptr, &tv);
        if(rc < 0)
            return tunnelFailed(strerror(errno));

        if(rc > 0 && FD_ISSET(sock_fwd_, &fds)) {
            ssize_t len = recv(sock_fwd_, buf, sizeof(buf), 0);
            if(len < 0) {
                char* msg = 0;
                libssh2_session_last_error(session, &msg, nullptr, 0);
                emit tunnelFailed(msg);
                free(msg);
                return;
            }
            else if (0 == len)
                return; // tunnelFailed("The client at %s:%d disconnected!\n", shost, sport);
            ssize_t n = 0;
            int i;
            do {
                i = libssh2_channel_write(channel, buf, len);
                if (i < 0) {
                    char* msg = 0;
                    libssh2_session_last_error(session, &msg, nullptr, 0);
                    emit tunnelFailed(msg);
                    free(msg);
                    return;
                }
                n += i;
            } while(i > 0 && n < len);
        }
        while(true) {
            ssize_t len = libssh2_channel_read(channel, buf, sizeof(buf));

            if (LIBSSH2_ERROR_EAGAIN == len)
                break;
            else if (len < 0)  {
                char* msg = 0;
                libssh2_session_last_error(session, &msg, nullptr, 0);
                emit tunnelFailed(msg);
                free(msg);
                return;
            }
            ssize_t n = 0;
            int i;
            while (n < len) {
                i = send(sock_fwd_, buf + n, len - n, 0);
                if (i <= 0) {
                    char* msg = 0;
                    libssh2_session_last_error(session, &msg, nullptr, 0);
                    emit tunnelFailed(msg);
                    free(msg);
                    return;
                }
                n += i;
            }
            if (libssh2_channel_eof(channel))
                return tunnelFailed("Disconnected by remote host");
        }
    }
}

int SshThread::connectToServer() {

    { // Connect to SSH server
        if(createSocket() &&
        createSession() &&
        authenticate() &&
        setupTunnel())
        routeTraffic();
    }





    shutdown();
    return 0;
}

int SshThread::shutdown(const char* msg, ...) {
    if(msg) {
        va_list args;
        va_start(args, msg);
        vfprintf(stderr, msg, args);
        va_end(args);
    }

    if(channel)
        libssh2_channel_free(channel);

#ifdef WIN32
    closesocket(sock_fwd_);
    closesocket(sock_listen_);
#else
    close(sock_fwd_);
    close(sock_listen_);
#endif

    if(session) {
        libssh2_session_disconnect(session, "Client disconnecting normally");
        libssh2_session_free(session);
    }

#ifdef WIN32
    closesocket(sock_);
#else
    close(sock_);
#endif
    return -1;
}
