/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SSHDBCONNECTION_H_
#define _SEQUELJOE_SSHDBCONNECTION_H_

#include "dbconnection.h"

#include "sshthread.h"

#include <QString>
#include <QObject>

class QThread;
class QSqlTableModel;
class QSettings;

class SshDbConnection :
        public DbConnection,
        public SshParameters
{
    Q_OBJECT
public:

    static constexpr int DEFAULT_SSH_PORT = 22;

    struct Params {
        QByteArray sshHost;
        short sshPort;
        QByteArray dbHost;
        short dbPort;
        QByteArray username;
        QByteArray password;
    };

    static constexpr const char* KEY_USE_SSH = "UseSsh";
    static constexpr const char* KEY_SSH_HOST = "SshHost";
    static constexpr const char* KEY_SSH_PORT = "SshPort";
    static constexpr const char* KEY_SSH_USER = "SshUser";
    static constexpr const char* KEY_SSH_PASS = "SshPass";
    static constexpr const char* KEY_SSH_KEY = "SshKeyPath";

    SshDbConnection(const QSettings& settings);
    virtual ~SshDbConnection();
    bool connect();

    virtual QByteArray remoteHost() const { return host_; }
    virtual short remotePort() const { return port_; }

    friend class SshThread;

private:
    Params params_;
    QThread* thread_;

    static short nSshLibUsers;

private slots:
    void beginDbConnect(int);
};

#endif // _SEQUELJOE_SSHDBCONNECTION_H_

