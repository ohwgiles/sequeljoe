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

class Driver;
class SshThread;

class QSqlDatabase;
class QSettings;
class QAbstractTableModel;
class QSqlTableModel;
class QSqlQueryModel;


struct SqlParams {
    QByteArray host;
    short port;
    QByteArray user;
    QByteArray pass;
    int type;
    QByteArray dbName;
};

struct SshParams {
    QByteArray sshHost;
    QByteArray sshPort;
    QByteArray remoteHost;
    int remotePort;
    QByteArray sshUser;
    bool useSshKey;
    QByteArray sshPass;
    QByteArray sshKeyPath;
};

class DbConnection : public QObject {
    Q_OBJECT
public:

    explicit DbConnection(const QSettings& settings);
    virtual ~DbConnection();

    virtual QSqlQueryModel* query(QString q, QSqlQueryModel* update = 0);

    QStringList databaseNames() const { return dbNames; }

    QString databaseName() const;

    QStringList tables() const;

public slots:
    virtual bool execQuery(QSqlQuery &q) const;

    void queryTableColumns(QString tableName, QObject* callbackOwner, const char* callbackName = "selectComplete");
    void queryTableIndices(QString tableName, QObject* callbackOwner, const char* callbackName = "describeComplete");
    void queryTableMetadata(QString tableName, QObject* callbackOwner, const char *callbackName = "describeComplete");
    void queryTableContent(QString query, QObject* callbackOwner, const char* callbackName = "selectComplete");
    void queryTableUpdate(QString query, QObject* callbackOwner, const char* callbackName = "updateComplete");
    QString queryCreateTable(QString tableName);
    void deleteTable(QString tableName);
    void createTable(QString tableName);
    void populateTables();
    void useDatabase(QString dbName);

    void start();
    void cleanup();

    Driver* sqlDriver() const { return driver; }

signals:
    void connectionSuccess();
    void connectionFailed(QString reason);
    void confirmUnknownHost(QString fingerprint, bool* ok);

    void queryExecuted(QString query, QString result) const;
    void databaseChanged(QString);

protected:

private slots:
    void openDatabase(QString host, int port);

private:
    void newConnection();

    void populateDatabases();


    static int nConnections;
    bool useSshTunnel;

    struct {
        SshThread* ssh;
        QThread* thread;
    } tunnel;

    Driver* driver;
    SqlParams sqlParams;
    SshParams sshParams;
    QStringList dbNames;
    QStringList tableNames;
};

#endif // _SEQUELJOE_DBCONNECTION_H_
