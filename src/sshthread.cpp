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

SshThread::SshThread(SshParameters& params) : params_(params)
{
    sock_ = -1;
    sock_listen_ = -1;
    sock_fwd_ = -1;
    if(nLibSshUsers++ == 0 && libssh2_init(0) != 0)
        throw new QException();//"libssh2 initialization failed");
}

SshThread::~SshThread()
{
    if(--nLibSshUsers == 0)
        libssh2_exit();
}

int SshThread::connectToServer() {
      // todo parametrize
      const char *keyfile1 = "/home/username/.ssh/id_rsa.pub";
      const char *keyfile2 = "/home/username/.ssh/id_rsa";


#ifdef WIN32
    char sockopt;
    WSADATA wsadata;

    WSAStartup(MAKEWORD(2,0), &wsadata);
#else
    int sockopt;
#endif

    //struct sockaddr_in sin;
    { // Connect to SSH server
        //sock_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        //sin.sin_family = AF_INET;
        qDebug() << params_.sshHost_.constData();
//        if (INADDR_NONE == (sin.sin_addr.s_addr = inet_addr(params_.host))) {
        //sin.sin_port = htons(params_.port);

        struct addrinfo* res;
        { // get address info
            struct addrinfo hints = {0};
            hints.ai_family = PF_INET; // | PF_INET6
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            if(getaddrinfo(params_.sshHost_.constData(), params_.sshPort_.constData(), &hints, &res) != 0) {
                perror("getaddrinfo");
                return -1;
            }
        }

        for(struct addrinfo* p = res; p; p = p->ai_next) {
            fprintf(stderr, "type: %d\n", p->ai_socktype);
            if((sock_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
                fprintf(stderr, "couldn't create socket: %s\n", strerror(errno));

                continue;
            }

            //p->ai_addr->sin_port = htons(params_.port);

            if(::connect(sock_, p->ai_addr, p->ai_addrlen) != 0) {
                fprintf(stderr, "couldn't connect: %s\n", strerror(errno));
                close(sock_);
                sock_ = -1;

                continue;

            }
            fprintf(stderr, "found one\n");
            break;
        }

        if(sock_ < 0) {
            fprintf(stderr, "failed to connect\n");
            return -1;
        }

        freeaddrinfo(res);
    }

    { // create a session
        if(nullptr == (session = libssh2_session_init())) {
            fprintf(stderr, "Could not initialize SSH session!\n");
            return -1;
        }

        // trade banners, keys, setup crypto, compression, MAC
        int rc = libssh2_session_handshake(session, sock_);
        if(rc) {
            char* msg;
            libssh2_session_last_error(session, &msg, NULL, 0);
            fprintf(stderr, "Error when starting up SSH session: %d (%s)\n", rc, msg);
            return -1;
        }
    }

    { // fingerprinting - from here we must redirect to shutdown rather than just returning
        const char* fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);

        fprintf(stderr, "Fingerprint: ");
        for(int i = 0; i < 20; i++)
            fprintf(stderr, "%02X ", (unsigned char)fingerprint[i]);
        fprintf(stderr, "\n");
        // TODO compare with known_hosts, popup dialog
    }

    { // authentication
        const char* userauthlist = libssh2_userauth_list(session, params_.sshUser_.constData(), params_.sshUser_.length());
        int auth = AUTH_NONE;
        fprintf(stderr, "Authentication methods: %s\n", userauthlist);
        if (strstr(userauthlist, "password"))
            auth |= AUTH_PASSWORD;
        if (strstr(userauthlist, "publickey"))
            auth |= AUTH_PUBLICKEY;

        /* check for options */
   //     if(argc > 8) {
   //         if ((auth & AUTH_PASSWORD) && !strcasecmp(argv[8], "-p"))
   //             auth = AUTH_PASSWORD;
   //         if ((auth & AUTH_PUBLICKEY) && !strcasecmp(argv[8], "-k"))
   //             auth = AUTH_PUBLICKEY;
   //     }

        if(params_.useSshKey_) {
            if (libssh2_userauth_publickey_fromfile_ex(session,
                    params_.sshUser_.constData(), params_.sshUser_.length(),
                           NULL, params_.sshKeyPath_.constData(), NULL))
                return shutdown("Authentication by public key failed!\n");
            fprintf(stderr, "Authentication by public key succeeded.\n");
        } else {
            if (libssh2_userauth_password_ex(session,
                    params_.sshUser_.constData(), params_.sshUser_.length(),
                    params_.sshPass_.constData(), params_.sshPass_.length(), NULL))
                return shutdown("Authentication by password failed.\n");
        }
        /*
        if (auth & AUTH_PASSWORD) {
            if (libssh2_userauth_password_ex(session,
                    params_.sshUser_.constData(), params_.sshUser_.length(),
                    params_.sshPass_.constData(), params_.sshPass_.length(), NULL))
                return shutdown("Authentication by password failed.\n");
        } else if (auth & AUTH_PUBLICKEY) {
            if (libssh2_userauth_publickey_fromfile_ex(session,
                    params_.sshUser_.constData(), params_.sshUser_.length(),
                           keyfile1, keyfile2, NULL))
                return shutdown("Authentication by public key failed!\n");
            fprintf(stderr, "Authentication by public key succeeded.\n");
        } else {
            return shutdown("No supported authentication methods found!\n");
        }*/
    }

struct sockaddr_in sin;
    { // create tunnel
        int localListenPort = localBoundPort++;
        sock_listen_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        sin.sin_family = AF_INET;
        sin.sin_port = htons(localListenPort);
        if (INADDR_NONE == (sin.sin_addr.s_addr = inet_addr("127.0.0.1")))
            return shutdown(strerror(errno));

        sockopt = 1;
        setsockopt(sock_listen_, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));
        socklen_t sinlen = sizeof(sin);
        qDebug() << "attempting to bind to local port " << localListenPort;
        if (-1 == bind(sock_listen_, (struct sockaddr *)&sin, sinlen))
            return shutdown(strerror(errno));
        if (-1 == listen(sock_listen_, 2))
            return shutdown(strerror(errno));

        fprintf(stderr, "Waiting for TCP connection on %s:%d...\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
        qDebug() << __PRETTY_FUNCTION__ << QThread::currentThreadId();

        // small race here, actually we shouldn't emit until accepted, but that's tricky :)
        emit sshTunnelOpened(localListenPort);

        if (-1 == (sock_fwd_ = accept(sock_listen_, (struct sockaddr *)&sin, &sinlen)))
            return shutdown(strerror(errno));
    }


    { // connected, route traffic
        const char* shost = inet_ntoa(sin.sin_addr);
        unsigned int sport = ntohs(sin.sin_port);

        fprintf(stderr, "Forwarding connection from %s:%d here to remote %s:%d\n", shost, sport, params_.remoteHost().constData(), params_.remotePort());

        channel = libssh2_channel_direct_tcpip_ex(session, params_.remoteHost().constData(), params_.remotePort(), shost, sport);
        if (!channel)
            return shutdown("Could not open the direct-tcpip channel!\n"
                    "(Note that this can be a problem at the server!"
                    " Please review the server logs.)\n");

        // Must use non-blocking IO hereafter due to the current libssh2 API
        libssh2_session_set_blocking(session, 0);

        // select loop
        struct timeval tv;
        fd_set fds;
        char buf[16384];

        for(;;) {
            FD_ZERO(&fds);
            FD_SET(sock_fwd_, &fds);
            tv.tv_sec = 0;
            tv.tv_usec = 100000;

            int rc = select(sock_fwd_ + 1, &fds, NULL, NULL, &tv);
            if (-1 == rc)
                return shutdown(strerror(errno));

            if (rc && FD_ISSET(sock_fwd_, &fds)) {
                ssize_t len = recv(sock_fwd_, buf, sizeof(buf), 0);
                if (len < 0)
                    return shutdown(strerror(errno));
                else if (0 == len)
                    return shutdown("The client at %s:%d disconnected!\n", shost, sport);
                ssize_t n = 0;
                int i;
                do {
                    i = libssh2_channel_write(channel, buf, len);
                    if (i < 0)
                        return shutdown("libssh2_channel_write: %d\n", i);
                    n += i;
                } while(i > 0 && n < len);
            }
            for(;;) {
                ssize_t len = libssh2_channel_read(channel, buf, sizeof(buf));

                if (LIBSSH2_ERROR_EAGAIN == len)
                    break;
                else if (len < 0)
                    return shutdown("libssh2_channel_read: %d", (int)len);
                ssize_t n = 0;
                int i;
                while (n < len) {
                    i = send(sock_fwd_, buf + n, len - n, 0);
                    if (i <= 0)
                        return shutdown(strerror(errno));
                    n += i;
                }
                if (libssh2_channel_eof(channel))
                    return shutdown("The server at %s:%d disconnected!\n", params_.remoteHost().constData(), params_.remotePort());
            }
        }
        // end select loop
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
#ifdef WIN32
    closesocket(sock_fwd_);
    closesocket(sock_listen_);
#else
    close(sock_fwd_);
    close(sock_listen_);
#endif
    if(channel)
        libssh2_channel_free(channel);

    libssh2_session_disconnect(session, "Client disconnecting normally");
    libssh2_session_free(session);

#ifdef WIN32
    closesocket(sock_);
#else
    close(sock_);
#endif
    return -1;
}
