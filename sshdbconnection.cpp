/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include <QThread>
#include <QException>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <qsqlquery.h>
#include <QSettings>
#include <QStringList>

#include "sshdbconnection.h"
#include "sshthread.h"

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
}

#include <QDebug>
bool SshDbConnection::connect()
{
    qDebug() << __PRETTY_FUNCTION__;
    thread_ = new QThread;
    SshThread* t = new SshThread(*this);//{ sshHost_.constData(), sshPort_, sshUser_.constData(), sshPass_.constData(), host_.constData(), port_ });
    t->moveToThread(thread_);
    QObject::connect(thread_, SIGNAL(started()), t, SLOT(connectToServer()));
    QObject::connect(t, SIGNAL(sshTunnelOpened(int)), this, SLOT(beginDbConnect(int)));
    qDebug() << "starting thread";
    thread_->start();
    return true; // ?
}
static char last_connect_name[] = "cs_name0";
void SshDbConnection::beginDbConnect(int port)
{
    qDebug() << __PRETTY_FUNCTION__ << QThread::currentThreadId();
    // do we need to wait until accept()?

    db_ = new QSqlDatabase(QSqlDatabase::addDatabase(type_, last_connect_name));
    last_connect_name[7]++;
qDebug() << "connect on " << port;
    db_->setHostName("127.0.0.1");
    db_->setPort(port);
    db_->setDatabaseName(dbName_);
    db_->setUserName(user_);
    db_->setPassword(pass_);
    bool ok = db_->open();
qDebug() << ok;
    populateDatabases();

    emit connectionSuccess();

}


