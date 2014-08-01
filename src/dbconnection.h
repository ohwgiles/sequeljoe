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

class QSqlDatabase;
class QSettings;
class QAbstractTableModel;
class QSqlTableModel;
class QSqlQueryModel;

class DbConnection : public QObject, public QSqlDatabase {
    Q_OBJECT
public:
    static constexpr int DEFAULT_SQL_PORT = 3306;
    static constexpr const char* KEY_HOST = "Host";
    static constexpr const char* KEY_DBNM = "DbName";
    static constexpr const char* KEY_PORT = "Port";
    static constexpr const char* KEY_TYPE = "Type";
    static constexpr const char* KEY_USER = "Username";
    static constexpr const char* KEY_PASS = "Password";

    static DbConnection* fromName(QString name);

    explicit DbConnection(const QSettings& settings);
    virtual ~DbConnection();

    virtual bool connect();
    virtual QAbstractTableModel* getTableModel(QString tableName);
    virtual QAbstractTableModel* getStructureModel(QString tableName);
    virtual QSqlQueryModel* query(QString q, QSqlQueryModel* update = 0);
    virtual bool execQuery(QSqlQuery& q);

    void deleteTable(QString tableName);
    void createTable(QString tableName);

    void populateDatabases();
    QStringList getDatabaseNames() const { return dbNames_; }
    QString getDatabaseName() const { return dbName_; }

public slots:
    void setDbName(QString name);

signals:
    void connectionSuccess();
    void queryExecuted(const QSqlQuery&);

protected:
    static int nConnections_;
    void newConnection();

    QByteArray host_;
    short port_;
    QByteArray user_;
    QByteArray pass_;
    QString type_;
    QByteArray dbName_;
    QStringList dbNames_;
    QHash<QString, QAbstractTableModel*> tableModels_;
    QHash<QString, QAbstractTableModel*> schemaModels_;
};

#endif // _SEQUELJOE_DBCONNECTION_H_
