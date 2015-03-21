#ifndef SQLDRIVER_H
#define SQLDRIVER_H

#include <QSqlDatabase>
#include "tabledata.h"

class QAbstractListModel;

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

class Driver : public QSqlDatabase {
public:
    virtual ~Driver(){}

    static QAbstractListModel *driverListModel();
    static Driver* createDriver(QString type);

    virtual QString quote(QVariant value);
    // fix non-virtualness of QSqlDatabase::open
    virtual bool open() { return QSqlDatabase::open(); }

    virtual QStringList databases() = 0;
    virtual TableData columns(QString table) = 0;
    virtual Indices indices(QString table) = 0;
    virtual TableMetadata metadata(QString table) = 0;
    virtual QStringList tableNames() = 0;
};

#endif // SQLDRIVER_H
