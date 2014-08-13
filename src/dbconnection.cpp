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
#include "notify.h"

#include <QSettings>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QStringList>
#include <QDebug>
#include <QSqlError>
#include <QApplication>

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
    dbFile_ = settings.value(DbConnection::KEY_FILE).toByteArray();
}

DbConnection::~DbConnection() {

    // resetting QSqlDatabase enables removeDatabase to succeed without
    // without complaining about the connection being in use
//    qDebug() << "resetting db";
//    qDebug() << "exiting dtor";
}

void DbConnection::cleanup() {
    for(QAbstractTableModel* m : tableModels_)
        delete m;
    for(QAbstractTableModel* m : schemaModels_)
        delete m;
    tableModels_.clear();
    schemaModels_.clear();
    close();

    QString name = connectionName();
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


bool DbConnection::execQuery(QSqlQuery& q) const {
    bool result = q.exec();
    //emit queryExecuted(q);
    QString msg;
    if(q.lastError().isValid())
        msg = "Error: " + q.lastError().text();
    else if(q.isSelect())
        msg = QString::number(q.size()) + " rows retrieved";
    else
        msg = QString::number(q.numRowsAffected()) + " rows affected";

    if(qApp->focusWindow() == 0) {
        notify->send("Query complete", msg.toLocal8Bit().constData());
    }
    emit queryExecuted(q.lastQuery(), msg);
    return result;
}
void DbConnection::queryTableMetadata(QString tableName, QObject* callbackOwner, const char* callbackName) {


    QSqlQuery q{*this};
    QVector<ColumnHeader> columns;
    int primaryKeyIndex = -1;
    QVariant recordEstimate;

    // todo subclass
    if(type_ == "QSQLITE") {
        q.prepare("PRAGMA table_info('" + tableName + "')");
        q.exec();
        while(q.next()) {
            if(q.value(5).toBool())
                primaryKeyIndex = columns.count();
            columns.append({q.value(1).toString()});
        }
        // too lazy to implement foreign key support. would probably require regex matching the sql column from sqlite_master


    } else {



        // primary key required if the row should be editable
        q.prepare(
            "select c.column_name, c.column_comment, c.column_key = 'PRI' as is_primary, k.referenced_table_name, k.referenced_column_name, t.table_rows "
            "from information_schema.columns as c "
            "inner join information_schema.tables as t "
            "on c.table_schema = t.table_schema and c.table_name = t.table_name "
            "left join information_schema.key_column_usage as k "
            "on c.table_schema = k.table_schema and c.table_name = k.table_name and c.column_name = k.column_name and referenced_column_name is not null "
            "where c.table_schema = '"+databaseName()+"' and c.table_name = '"+tableName+"'"
        );
        q.exec();
        // reserve columns optimisation possible?
        while(q.next()) {
            if(q.value(2).toBool())
                primaryKeyIndex = columns.count();
            columns.append({q.value(0).toString(),
                             q.value(1).toString(),
                             q.value(3).toString(),
                             q.value(4).toString()});
            recordEstimate = q.value(5).toInt();
        }

        //totalRecords_ = recordEstimate.toInt();
    }
    QMetaObject::invokeMethod(callbackOwner, callbackName, Qt::QueuedConnection, Q_ARG(QVector<ColumnHeader>, columns), Q_ARG(int, recordEstimate.toInt()), Q_ARG(int, primaryKeyIndex));

}
#include <QSqlRecord>
#include "tabledata.h"
void DbConnection::queryTableContent(QString query, QObject* callbackOwner, const char* callbackName) {
    QSqlQuery q(*this);
    TableData data;
    data.reserve(q.size());
    q.prepare(query);
    execQuery(q);
    while(q.next()) {
        QVector<QVariant> row;
        row.resize(q.record().count());
        for(int i = 0; i < q.record().count(); ++i)
            row[i] = q.value(i);
        data.append(row);
    }
    //qDebug() << data;
    QMetaObject::invokeMethod(callbackOwner, callbackName, Qt::QueuedConnection, Q_ARG(TableData, data));
}
void DbConnection::queryTableColumns(QString tableName, QObject* callbackOwner, const char* callbackName) {
    QVector<QVector<QVariant>> data;


    if(type_ == "QSQLITE") {
        QSqlQuery q("PRAGMA table_info(" + tableName + ")", *this);
        while(q.next()) {
            SqlColumn c;
            c.resize(SCHEMA_NUM_FIELDS);
            c[SCHEMA_NAME] = q.value(1).toString();
            c[SCHEMA_TYPE] = q.value(2).toString().toUpper();
            c[SCHEMA_UNSIGNED] = (q.value(2).toString().contains("unsigned", Qt::CaseInsensitive));
            c[SCHEMA_LENGTH] = 0;
            c[SCHEMA_NULL] = bool(q.value(3).toInt());
            c[SCHEMA_KEY] = "";
            c[SCHEMA_DEFAULT] = q.value(4).toString();
            c[SCHEMA_EXTRA] = "";
            c[SCHEMA_COLLATION] = "";
            c[SCHEMA_COMMENT] = "";
            data.append(c);
        }

    } else {
        QSqlQuery q("SHOW FULL COLUMNS FROM " + tableName, *this);

        data.reserve(q.size());

        QRegExp typeRegexp("(\\w+)\\(([\\w,]+)\\)\\s*(\\w*)");
        while(q.next()) {
            SqlColumn c;
            c.resize(SCHEMA_NUM_FIELDS);
            c[SCHEMA_NAME] = q.value(0).toString();
            if(typeRegexp.exactMatch(q.value(1).toString())) {
                c[SCHEMA_TYPE] = typeRegexp.cap(1).toUpper();
                c[SCHEMA_LENGTH] = typeRegexp.cap(2);
                c[SCHEMA_UNSIGNED] = (typeRegexp.cap(3) == "unsigned");
            } else
                c[SCHEMA_TYPE] = q.value(1).toString().toUpper(); //e.g. TEXT has no length or unsigned
            c[SCHEMA_NULL] = (q.value(3).toString() == "YES");
            c[SCHEMA_KEY] = q.value(4).toString();
            c[SCHEMA_DEFAULT] = q.value(5).toString();
            c[SCHEMA_EXTRA] = q.value(6).toString();
            c[SCHEMA_COLLATION] = q.value(2).toString();
            c[SCHEMA_COMMENT] = q.value(8).toString();
            data.append(c);
        }
    }

    QMetaObject::invokeMethod(callbackOwner, callbackName, Qt::QueuedConnection, Q_ARG(QVector<QVector<QVariant>>, data));
}

void DbConnection::queryTableUpdate(QString query, QObject *callbackOwner, const char *callbackName) {
    QSqlQuery q(*this);
    q.prepare(query);
    bool result = execQuery(q);
    QMetaObject::invokeMethod(callbackOwner, callbackName, Qt::QueuedConnection, Q_ARG(bool, result), Q_ARG(int, q.lastInsertId().toInt()));
}

void DbConnection::populateDatabases() {
    dbNames_.clear();
    QSqlQuery query(*this);
    query.prepare("SHOW DATABASES");
    execQuery(query);
    while(query.next()) {
        dbNames_ << query.value(0).toString();
    }
}

void DbConnection::createTable(QString tableName) {
    QSqlQuery query(*this);
    query.prepare("CREATE TABLE `" + tableName + "` (`id` INT UNSIGNED PRIMARY KEY NOT NULL AUTO_INCREMENT)");
    execQuery(query);
}

void DbConnection::deleteTable(QString tableName) {
    QSqlQuery query(*this);
    query.prepare("DROP TABLE `" + tableName + "`");
    execQuery(query);
}

void DbConnection::newConnection() {
    QString name = "connection_" + QString::number(nConnections_++);
    *((QSqlDatabase*) this) = QSqlDatabase::addDatabase(type_, name);
}

bool DbConnection::connect() {
    newConnection();
    setHostName(host_);
    setPort(port_);
    setDatabaseName(dbFile_.isEmpty() ? dbName_ : dbFile_);
    setUserName(user_);
    setPassword(pass_);
    bool ok = this->open();
    if(ok) {
        if(dbFile_.isEmpty())
            populateDatabases();
        if(!dbFile_.isEmpty() || !dbName_.isEmpty())
            populateTables();
        emit connectionSuccess();
        return true;
    } else
        qDebug() << "connect failed";
    return false;
}

void DbConnection::useDatabase(QString dbName) {
    QSqlQuery query(*this);
    query.prepare("USE `" + dbName + "`");
    execQuery(query);
    setDatabaseName(dbName);
    populateTables();
    emit databaseChanged(dbName);
}
void DbConnection::QueryInterface(std::function<void(QSqlQuery)> fn) {
    QSqlQuery q;
    fn(q);
}

void DbConnection::populateTables() {
    QSqlQuery query(*this);
    // todo abstract
    if(type_ == "QSQLITE")
        query.prepare("select name from sqlite_master where type='table'");
    else
        query.prepare("SHOW TABLES");
    execQuery(query);
    QStringList result;
    while(query.next())
        result << query.value(0).toString();
    tables_ = result;
}
