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
#include <QSqlRecord>

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
    TableMetadata metadata;

    // todo subclass
    if(type_ == "QSQLITE") {
        q.prepare("PRAGMA table_info('" + tableName + "')");
        q.exec();
        if(q.size()) metadata.resize(q.record().count());
        int i = 0;
        while(q.next()) {
            if(q.value(5).toBool())
                metadata.primaryKeyColumn = i;
            i++;
            //columns.append({q.value(1).toString()});
        }
        // too lazy to implement foreign key support. would probably require regex matching the sql column from sqlite_master


    } else {



        // primary key required if the row should be editable
        q.prepare(
            "select c.column_name, c.column_comment, c.column_key = 'PRI' as is_primary, k.referenced_table_name, k.referenced_column_name, t.table_rows, c.data_type "
            "from information_schema.columns as c "
            "inner join information_schema.tables as t "
            "on c.table_schema = t.table_schema and c.table_name = t.table_name "
            "left join information_schema.key_column_usage as k "
            "on c.table_schema = k.table_schema and c.table_name = k.table_name and c.column_name = k.column_name and referenced_column_name is not null "
            "where c.table_schema = '"+databaseName()+"' and c.table_name = '"+tableName+"'"
            "order by c.ordinal_position"
        );
        q.exec();
        // reserve columns optimisation possible?
        if(q.first()) {
            int i = 0;
            qDebug() << q.record().count();
            qDebug() << q.size();
            metadata.resize(q.size());
            do {
                if(q.value(2).toBool())
                    metadata.primaryKeyColumn = i;
                metadata.columnNames[i] = q.value(0).toString();
                metadata.columnTypes[i] = q.value(6).toString();
                metadata.columnComments[i] = q.value(1).toString();
                metadata.foreignKeyTables[i] = q.value(3).toString();
                metadata.foreignKeyColumns[i] = q.value(4).toString();
                //recordEstimate = q.value(5).toInt();
                i++;

            } while(q.next());
        }

        //totalRecords_ = recordEstimate.toInt();
    }
    QMetaObject::invokeMethod(callbackOwner, callbackName, Qt::QueuedConnection, Q_ARG(TableMetadata, metadata));

}

#include <QSqlRecord>
void DbConnection::queryTableIndices(QString tableName, QObject *callbackOwner, const char *callbackName) {
    QSqlQuery q{*this};

    Indices indices;
    q.prepare("show index from " + tableName);
    execQuery(q);
    Index currentIndex;
    QString currentIndexName;
    while(q.next()) {
        if(currentIndex.name != q.value(2).toString()) {
            if(!currentIndex.name.isNull()) {
                indices.append(currentIndex);
                currentIndex.members.clear();
            }
            currentIndex.name = q.value(2).toString();
        }
        currentIndex.members.append({
                                        q.value(4).toString(),
                                        q.value(3).toInt(),
                                        !q.value(1).toBool(),
                                        0

                                    });

    }
    if(!currentIndex.name.isNull())
        indices.append(currentIndex);
    //qDebug() << data;
    QMetaObject::invokeMethod(callbackOwner, callbackName, Qt::QueuedConnection, Q_ARG(Indices, indices));
}

#include "tabledata.h"
#include <QSqlResult>
void DbConnection::queryTableContent(QString query, QObject* callbackOwner, const char* callbackName) {
    QSqlQuery q(*this);
    TableData data;
    data.reserve(q.size());
    q.prepare(query);
    execQuery(q);
    if(q.first()) {
        data.columnNames.resize(q.record().count());
        for(int i = 0; i < q.record().count(); ++i) {
//            qDebug() << "adding column name " << q.record().fieldName(i);
            data.columnNames[i] = q.record().fieldName(i).trimmed();
        }
        do {
            QVector<QVariant> row;
            row.resize(q.record().count());
            for(int i = 0; i < q.record().count(); ++i)
                row[i] = q.value(i);
            data.append(row);
        } while(q.next());
    }
    //qDebug() << data;
    QMetaObject::invokeMethod(callbackOwner, callbackName, Qt::QueuedConnection, Q_ARG(TableData, data));
}
void DbConnection::queryTableColumns(QString tableName, QObject* callbackOwner, const char* callbackName) {
    TableData data;
QSqlQuery q{*this};

    if(type_ == "QSQLITE") {
        q.prepare("PRAGMA table_info(" + tableName + ")");
        q.exec();
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
            c[SCHEMA_COMMENT] = "";
            c[SCHEMA_FOREIGNKEY] = "";
            data.append(c);
        }

    } else {
        q.prepare("select c.column_name, c.column_type, c.is_nullable, c.column_key, c.column_default, c.extra, c.column_comment, k.referenced_table_name, k.referenced_column_name "
                  "from information_schema.columns as c "
                  "left join information_schema.key_column_usage as k "
                  "on c.table_schema = k.table_schema and c.table_name = k.table_name and c.column_name = k.column_name and referenced_column_name is not null "
                  "where c.table_schema = '"+databaseName()+"' and c.table_name = '"+tableName+"'");
        q.exec();

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
            c[SCHEMA_NULL] = (q.value(2).toString() == "YES");
            c[SCHEMA_KEY] = q.value(3).toString();
            c[SCHEMA_DEFAULT] = q.value(4).toString();
            c[SCHEMA_EXTRA] = q.value(5).toString();
            //c[SCHEMA_COLLATION] = q.value(2).toString();
            c[SCHEMA_COMMENT] = q.value(6).toString();
            c[SCHEMA_FOREIGNKEY] = q.value(8).isNull() ? QVariant() : (q.value(7).toString() + '.' + q.value(8).toString());
            data.append(c);
        }
    }

    QMetaObject::invokeMethod(callbackOwner, callbackName, Qt::QueuedConnection, Q_ARG(TableData, data));
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
    emit connectionFailed(this->lastError().text());
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
