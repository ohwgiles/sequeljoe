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

class QSqlDatabase;
class QSettings;
class QSqlTableModel;
class QSqlQueryModel;

class DbConnection : public QObject
{
    Q_OBJECT
public:
    static constexpr int DEFAULT_SQL_PORT = 3306;
    DbConnection(const QSettings& settings);
    virtual ~DbConnection();
    virtual bool connect() = 0;
    static DbConnection* fromName(QString name);

    virtual QStringList getTableNames();
    virtual QSqlTableModel* getTableModel(QString tableName);
    virtual QSqlQueryModel* getStructureModel(QString tableName);
    virtual QSqlQueryModel* query(QString q, QSqlQueryModel* update = 0);
    static constexpr const char* KEY_HOST = "Host";
    static constexpr const char* KEY_DBNM = "DbName";
    static constexpr const char* KEY_PORT = "Port";
    static constexpr const char* KEY_TYPE = "Type";
    static constexpr const char* KEY_USER = "Username";
    static constexpr const char* KEY_PASS = "Password";

    void close();
    void populateDatabases();
QStringList getDatabaseNames() const { return dbNames_; }

public slots:
    void setDbName(QString name);
signals:
    void connectionSuccess();

protected:
    QByteArray host_;
    short port_;
    QByteArray user_;
    QByteArray pass_;
    QString type_;
    QByteArray dbName_;
    QSqlDatabase* db_;
QStringList dbNames_;
    QHash<QString, QSqlTableModel*> tableModels_;
    QHash<QString, QSqlQueryModel*> schemaModels_;
};

#endif // _SEQUELJOE_DBCONNECTION_H_
