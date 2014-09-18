/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_DBCONNECTION_H_
#define _SEQUELJOE_DBCONNECTION_H_

#include <QString>
#include <QObject>
#include <QHash>
#include <QStringList>
#include <QSqlDatabase>
#include <functional>

//del
#include <QSqlQuery>

class QSqlDatabase;
class QSettings;
class QAbstractTableModel;
class QSqlTableModel;
class QSqlQueryModel;
enum {
    SCHEMA_NAME = 0,
    SCHEMA_TYPE,
    SCHEMA_LENGTH,
    SCHEMA_UNSIGNED,
    SCHEMA_NULL,
    SCHEMA_KEY,
    SCHEMA_DEFAULT,
    SCHEMA_EXTRA,
    SCHEMA_FOREIGNKEY,
    SCHEMA_COMMENT,

    SCHEMA_NUM_FIELDS
};
class DbConnection : public QObject, public QSqlDatabase {
    Q_OBJECT
public:
    static constexpr int DEFAULT_SQL_PORT = 3306;
    static constexpr const char* KEY_HOST = "Host";
    static constexpr const char* KEY_DBNM = "DbName";
    static constexpr const char* KEY_FILE = "DbFile";
    static constexpr const char* KEY_PORT = "Port";
    static constexpr const char* KEY_TYPE = "Type";
    static constexpr const char* KEY_USER = "Username";
    static constexpr const char* KEY_PASS = "Password";

    static DbConnection* fromName(QString name);

    explicit DbConnection(const QSettings& settings);
    virtual ~DbConnection();

    virtual QSqlQueryModel* query(QString q, QSqlQueryModel* update = 0);


    void populateDatabases();
    QStringList getDatabaseNames() const { return dbNames_; }
    QString getDatabaseName() const { return dbName_; }


    QStringList tables() const { return tables_; }
public slots:
    virtual bool execQuery(QSqlQuery &q) const;


    void queryTableColumns(QString tableName, QObject* callbackOwner, const char* callbackName = "describeComplete");
    void queryTableIndices(QString tableName, QObject* callbackOwner, const char* callbackName = "describeComplete");
    void queryTableMetadata(QString tableName, QObject* callbackOwner, const char *callbackName = "describeComplete");
    void queryTableContent(QString query, QObject* callbackOwner, const char* callbackName = "selectComplete");
    void queryTableUpdate(QString query, QObject* callbackOwner, const char* callbackName = "updateComplete");

    void cleanup();

    void deleteTable(QString tableName);
    void createTable(QString tableName);
    void useDatabase(QString dbName);
//    void setDbName(QString name);
    virtual bool connect();

    void QueryInterface(std::function<void(QSqlQuery)> fn);

signals:
    void connectionSuccess();
    void connectionFailed(QString reason);
    void confirmUnknownHost(QString fingerprint, bool* ok);

    void queryExecuted(QString query, QString result) const;
    void databaseChanged(QString);

protected:
    static int nConnections_;
    void newConnection();

    void populateTables();

    QByteArray host_;
    short port_;
    QByteArray user_;
    QByteArray pass_;
    QString type_;
    QByteArray dbName_;
    QByteArray dbFile_;
    QStringList dbNames_;
    QStringList tables_;
};

#endif // _SEQUELJOE_DBCONNECTION_H_
