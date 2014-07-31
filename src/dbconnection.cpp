/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "dbconnection.h"

#include "sshdbconnection.h"
#include "sqlschemamodel.h"
#include "sqlcontentmodel.h"

#include <QSettings>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QStringList>

int DbConnection::nConnections_ = 0;

DbConnection::DbConnection(const QSettings &settings) {
    host_ = settings.value(DbConnection::KEY_HOST).toByteArray();
    port_ = settings.value(DbConnection::KEY_PORT).toInt();
    if(port_ == 0)
        port_ = DEFAULT_SQL_PORT;
    user_ = settings.value(DbConnection::KEY_USER).toByteArray();
    pass_ = settings.value(DbConnection::KEY_PASS).toByteArray();
    type_ = settings.value(DbConnection::KEY_TYPE).toString();
    dbName_ = settings.value(DbConnection::KEY_DBNM).toByteArray();
}

DbConnection::~DbConnection() {
    for(QAbstractTableModel* m : tableModels_)
        delete m;
    for(QAbstractTableModel* m : schemaModels_)
        delete m;
    close();

    QString name = connectionName();
    // resetting QSqlDatabase enables removeDatabase to succeed without
    // without complaining about the connection being in use
    *((QSqlDatabase*) this) = QSqlDatabase();
    QSqlDatabase::removeDatabase(name);
}

DbConnection* DbConnection::fromName(QString name) {
    QSettings s;
    s.beginGroup(name);
    if(s.value(SshDbConnection::KEY_USE_SSH).toBool())
        return new SshDbConnection(s);
    return new DbConnection(s);
}

QSqlQueryModel* DbConnection::query(QString q, QSqlQueryModel* update) {
    if(!update)
        update = new QSqlQueryModel;
    update->setQuery(q, *this);
    return update;
}

QAbstractTableModel *DbConnection::getTableModel(QString tableName) {
    if(!tableModels_.contains(databaseName() + tableName)) {
        QAbstractTableModel* model = new SqlContentModel(this, tableName);
        tableModels_[databaseName()+tableName] = model;
    }
    return tableModels_[databaseName()+tableName];
}

QAbstractTableModel* DbConnection::getStructureModel(QString tableName) {
    if(!schemaModels_.contains(tableName)) {
        QAbstractTableModel *model = new SqlSchemaModel(this, tableName);
        schemaModels_[tableName] = model;
    }
    return schemaModels_[tableName];
}

void DbConnection::populateDatabases() {
    dbNames_.clear();
    QSqlQuery query(*this);
    query.exec("SHOW DATABASES");
    while(query.next())
        dbNames_ << query.value(0).toString();
}

static char last_connect_name[] = "cs_name0";

void DbConnection::newConnection() {
    QString name = "connection_" + QString::number(nConnections_++);
    *((QSqlDatabase*) this) = QSqlDatabase::addDatabase(type_, name);
}

bool DbConnection::connect() {
    newConnection();
    setHostName(host_);
    setPort(port_);
    setDatabaseName(dbName_);
    setUserName(user_);
    setPassword(pass_);
    bool ok = this->open();
    if(ok) {
        populateDatabases();
        emit connectionSuccess();
    }
}

void DbConnection::setDbName(QString name) {
    dbName_ = name.toLocal8Bit();
    setDatabaseName(name);
    open();
}
