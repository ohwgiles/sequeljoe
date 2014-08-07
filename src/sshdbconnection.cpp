/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "sshdbconnection.h"

#include "sshthread.h"

#include <QThread>
#include <QException>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <qsqlquery.h>
#include <QSettings>
#include <QStringList>
#include <QDebug>

SshDbConnection::SshDbConnection(const QSettings &settings) :
    DbConnection(settings)
{
    sshHost_ = settings.value(KEY_SSH_HOST).toByteArray();
    sshPort_ = settings.value(KEY_SSH_PORT).toInt() == 0 ?
                QString::number(DEFAULT_SSH_PORT).toLocal8Bit() :
                settings.value(KEY_SSH_PORT).toByteArray();
    sshUser_ = settings.value(KEY_SSH_USER).toByteArray();
    useSshKey_ = settings.contains(KEY_SSH_KEY);
    sshKeyPath_ = settings.value(KEY_SSH_KEY).toByteArray();
    sshPass_ = settings.value(KEY_SSH_PASS).toByteArray();
}

SshDbConnection::~SshDbConnection()
{
    delete tunnel_;
    thread_->exit();
    thread_->wait();
    delete thread_;
}

bool SshDbConnection::connect() {
    thread_ = new QThread;
    tunnel_ = new SshThread(*this);
    tunnel_->moveToThread(thread_);
    QObject::connect(thread_, SIGNAL(started()), tunnel_, SLOT(connectToServer()));
    QObject::connect(tunnel_, SIGNAL(sshTunnelOpened(int)), this, SLOT(beginDbConnect(int)));
    thread_->start();
    return true;
}

void SshDbConnection::beginDbConnect(int port) {
    newConnection();
    setHostName("127.0.0.1");
    setPort(port);
    setDatabaseName(dbName_);
    setUserName(user_);
    setPassword(pass_);
    bool ok = this->open();
    if(ok) {
        qDebug() << "db name : " << dbName_;
        populateDatabases();
        if(!dbName_.isEmpty())
            populateTables();
        emit connectionSuccess();
    } else
        qDebug() << "connect failed";
}


