/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include <QSettings>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QStringList>
#include "dbconnection.h"
#include "sshdbconnection.h"

DbConnection::DbConnection(const QSettings &settings)
{
    host_ = settings.value(DbConnection::KEY_HOST).toByteArray();
    port_ = settings.value(DbConnection::KEY_PORT).toInt();
    if(port_ == 0)
        port_ = DEFAULT_SQL_PORT;
    user_ = settings.value(DbConnection::KEY_USER).toByteArray();
    pass_ = settings.value(DbConnection::KEY_PASS).toByteArray();
    type_ = settings.value(DbConnection::KEY_TYPE).toString();
    dbName_ = settings.value(DbConnection::KEY_DBNM).toByteArray();
}

void DbConnection::close() { if(db_->isOpen()) db_->close(); }

DbConnection::~DbConnection()
{
    for(QSqlTableModel* m : tableModels_)
        delete m;
    for(QSqlQueryModel* m : schemaModels_)
        delete m;
    db_->close();
    QSqlDatabase::removeDatabase(db_->connectionName());
}

DbConnection* DbConnection::fromName(QString name) {
    QSettings s;
    s.beginGroup(name);
    if(s.value(SshDbConnection::KEY_USE_SSH).toBool()) {
        return new SshDbConnection(s);
    }
    return nullptr;
}


 QSqlQueryModel* DbConnection::query(QString q, QSqlQueryModel* update) {
     if(!update)
         update = new QSqlQueryModel;
     update->setQuery(q, *db_);
     return update;
 }
#include <QDebug>
 QStringList DbConnection::getTableNames() {
     return db_->tables();
 }

 QSqlTableModel* DbConnection::getTableModel(QString tableName) {
     if(!tableModels_.contains(tableName)) {
         QSqlTableModel* model = new QSqlTableModel(0, *db_);
         model->setTable(tableName);
         model->setEditStrategy(QSqlTableModel::OnFieldChange);
         model->select();
         tableModels_[tableName] = model;
     }
     return tableModels_[tableName];
 }

 QSqlQueryModel* DbConnection::getStructureModel(QString tableName) {
     if(!schemaModels_.contains(tableName)) {
     QSqlQueryModel *model = new QSqlQueryModel;
        model->setQuery("DESCRIBE " + tableName, *db_);
        schemaModels_[tableName] = model;
     }
        //model->setHeaderData(0, Qt::Horizontal, tr("Name"));
        //model->setHeaderData(1, Qt::Horizontal, tr("Salary"));
        return schemaModels_[tableName];

}

 void DbConnection::populateDatabases()
 {
     dbNames_.clear();
     QSqlQuery query(*db_);
     query.exec("SHOW DATABASES");
     while(query.next())
         dbNames_ << query.value(0).toString();
 }


void DbConnection::setDbName(QString name) {
    dbName_ = name.toLocal8Bit();
    return;
    db_->setDatabaseName(name);
//db_->
    db_->open();
}
