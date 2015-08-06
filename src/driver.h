#ifndef SQLDRIVER_H
#define SQLDRIVER_H

#include <QSqlDatabase>
#include "tabledata.h"

class QSqlQuery;
class QAbstractListModel;

class Driver : public QSqlDatabase {
public:
    virtual ~Driver(){}

    static QAbstractListModel *driverListModel(QObject* parent = 0);
    static Driver* createDriver(QString type);

    virtual QString quote(QVariant value);
    // fix non-virtualness of QSqlDatabase::open
    virtual bool open() { return QSqlDatabase::open(); }

    virtual QStringList databases() = 0;
    virtual void columns(Schema& res, QString table) = 0;
    virtual TableMetadata metadata(QString table) = 0;
    virtual QStringList tableNames() = 0;
    virtual QString createTableQuery(QString table) = 0;
    virtual int countRows(QSqlQuery& q) const;
};

#endif // SQLDRIVER_H
