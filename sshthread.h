/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SSHTHREAD_H_
#define _SEQUELJOE_SSHTHREAD_H_

// no headers: this must be included after sshdbconnection.h
struct _LIBSSH2_SESSION;
struct _LIBSSH2_CHANNEL;

#include <QByteArray>

struct SshParameters {
    QByteArray sshHost_;
    QByteArray sshPort_;
    QByteArray sshUser_;
    bool useSshKey_;
    QByteArray sshPass_;
    QByteArray sshKeyPath_;
    virtual QByteArray remoteHost() const = 0;
    virtual short remotePort() const = 0;
};

class SshThread : public QObject
{
    Q_OBJECT
public:

    struct Parameters {
        const char* host;
        short port;
        const char* user;
        const char* pass;
        const char* remote_host;
        short remote_port;
    };

    SshThread(SshParameters& params);
    ~SshThread();

public slots:
    int connectToServer();
signals:
    void sshTunnelOpened(int);

private:
    int sock_, sock_fwd_, sock_listen_;
    _LIBSSH2_SESSION *session;
    _LIBSSH2_CHANNEL *channel = NULL;

    int shutdown(const char *msg = nullptr, ...);

    const SshParameters& params_;
    static int nLibSshUsers;
    static int localBoundPort;
};

#endif // _SEQUELJOE_SSHTHREAD_H_
