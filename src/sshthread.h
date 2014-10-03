/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SSHTHREAD_H_
#define _SEQUELJOE_SSHTHREAD_H_

struct _LIBSSH2_SESSION;
struct _LIBSSH2_CHANNEL;

#include <QByteArray>
#include <QObject>

class SshParams;

class SshThread : public QObject
{
    Q_OBJECT
public:
    SshThread(SshParams& params);
    virtual ~SshThread();

public slots:
    void connectToServer();

signals:
    void sshTunnelOpened(int);
    void tunnelFailed(QString);
    void confirmUnknownHost(QString fingerprint, bool* ok); //< should be blocked

private:
    bool createSocket();
    bool createSession();
    bool authenticate();
    bool setupTunnel();
    void routeTraffic();

    int sock;
    int sockFwd;
    int sockListen;

    _LIBSSH2_SESSION *session = nullptr;
    _LIBSSH2_CHANNEL *channel = nullptr;

    const SshParams& params;

    static int nLibSshUsers;
    static int localBoundPort;
};

#endif // _SEQUELJOE_SSHTHREAD_H_
