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

    enum DriverType {
        SQLITE,
        MYSQL
    };

    static QAbstractListModel *driverListModel();
    static Driver* createDriver(DriverType type);

    virtual QString quote(QVariant value);

    virtual TableData columns(QString table) = 0;
    virtual Indices indices(QString table) = 0;
    virtual TableMetadata metadata(QString table) = 0;
    virtual QStringList tableNames() = 0;
    virtual QString driverCode() const = 0;
};

#endif // SQLDRIVER_H
