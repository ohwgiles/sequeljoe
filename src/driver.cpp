#include "driver.h"
#include "roles.h"
#include "foreignkey.h"

#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QSqlField>

class SqlDriverList : public QAbstractListModel {
public:
    SqlDriverList() : QAbstractListModel()
    {
        QSqlDatabase db;
        QStringList availableDrivers = db.drivers();
        if(availableDrivers.contains("QSQLITE"))
            typeLabels.append({Driver::SQLITE, "SQLite"});
        if(availableDrivers.contains("QMYSQL"))
            typeLabels.append({Driver::MYSQL, "MySQL"});
    }

    int rowCount(const QModelIndex &parent) const {
        return typeLabels.count();
    }

    QVariant data(const QModelIndex &index, int role) const {
        if(index.isValid()) {
            if(role == Qt::DisplayRole)
                return typeLabels.at(index.row()).second;

            if(role == Qt::EditRole)
                return typeLabels.at(index.row()).first;

            if(role == DatabaseIsFileRole)
                return typeLabels.at(index.row()).first == Driver::SQLITE;
        }
        return QVariant();
    }

private:
    QVector<QPair<Driver::DriverType, QString>> typeLabels;
};

class MySqlDriver : public Driver {
public:
    virtual TableData columns(QString table) override {
        TableData data;
        QSqlQuery q{*this};

        q.prepare("select c.column_name, c.column_type, c.is_nullable, c.column_key, c.column_default, c.extra, c.column_comment, k.referenced_table_name, k.referenced_column_name, k.constraint_name "
                  "from information_schema.columns as c "
                  "left join information_schema.key_column_usage as k "
                  "on c.table_schema = k.table_schema and c.table_name = k.table_name and c.column_name = k.column_name and referenced_column_name is not null "
                  "where c.table_schema = '"+databaseName()+"' and c.table_name = '"+table+"'");
        q.exec();

        data.reserve(q.size());

        QRegExp typeRegexp("(\\w+)\\(([\\w,]+)\\)\\s*(\\w*)");
        while(q.next()) {
            QVector<QVariant> c;
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
            c[SCHEMA_COMMENT] = q.value(6).toString();
//QVariant v;
//ForeignKey fk;
//fk.table = q.value(7).toString();
//fk.column = q.value(8).toString();
//v.setValue(fk);
//c[SCHEMA_FOREIGNKEY] = v;
            c[SCHEMA_FOREIGNKEY] = QVariant::fromValue<ForeignKey>({q.value(7).toString(),q.value(8).toString(),q.value(9).toString()});
            data.append(c);
        }
        return data;
    }

    virtual Indices indices(QString table) override {
        QSqlQuery q{*this};

        Indices indices;
        q.prepare("show index from " + table);
        q.exec();
        Index currentIndex;
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

        return indices;
    }

    virtual TableMetadata metadata(QString table) override {
        QSqlQuery q{*this};
        TableMetadata metadata;
        q.prepare(
            "select c.column_name, c.column_comment, c.column_key = 'PRI' as is_primary, k.constraint_name, k.referenced_table_name, k.referenced_column_name, t.table_rows, c.data_type "
            "from information_schema.columns as c "
            "inner join information_schema.tables as t "
            "on c.table_schema = t.table_schema and c.table_name = t.table_name "
            "left join information_schema.key_column_usage as k "
            "on c.table_schema = k.table_schema and c.table_name = k.table_name and c.column_name = k.column_name and referenced_column_name is not null "
            "where c.table_schema = '"+databaseName()+"' and c.table_name = '"+table+"'"
            "order by c.ordinal_position"
        );
        q.exec();
        if(q.first()) {
            int i = 0;
            metadata.resize(q.size());
            metadata.numRows = q.value(6).toInt();
            do {
                if(q.value(2).toBool())
                    metadata.primaryKeyColumn = i;
                metadata.columnNames[i] = q.value(0).toString();
                metadata.columnTypes[i] = q.value(7).toString();
                metadata.columnComments[i] = q.value(1).toString();
                metadata.foreignKeys[i] = {q.value(4).toString(), q.value(5).toString(), q.value(3).toString() };
                i++;

            } while(q.next());
        }
        return metadata;
    }

    virtual QStringList tableNames() override {
        QSqlQuery query(*this);
        query.prepare("SHOW TABLES");
        query.exec();
        QStringList result;
        while(query.next())
            result << query.value(0).toString();
        return result;
    }

    virtual QString driverCode() const override {
        return "QMYSQL";
    }

};

class SqliteDriver : public Driver {
public:
    virtual TableData columns(QString table) override {
        TableData data;
        QSqlQuery q{*this};
        q.prepare("PRAGMA table_info(" + table + ")");
        q.exec();
        while(q.next()) {
            QVector<QVariant> c;
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

        return data;
    }

    virtual Indices indices(QString table) override {
        // not implemented
        return Indices{};
    }

    virtual TableMetadata metadata(QString table) override {
        QSqlQuery q{*this};
        TableMetadata metadata;
        // too lazy to implement foreign key support. would probably require regex matching the sql column from sqlite_master

        q.prepare("PRAGMA table_info('" + table + "')");
        q.exec();
        if(q.size())
            metadata.resize(q.record().count());
        int i = 0;
        while(q.next()) {
            if(q.value(5).toBool())
                metadata.primaryKeyColumn = i;
            i++;
        }

        return metadata;
    }

    virtual QStringList tableNames() override {
        QSqlQuery query(*this);
        query.prepare("select name from sqlite_master where type='table'");
        query.exec();
        QStringList result;
        while(query.next())
            result << query.value(0).toString();
        return result;
    }

    virtual QString driverCode() const override {
        return "QSQLITE";
    }
};


QAbstractListModel* Driver::driverListModel() {
    return new SqlDriverList();
}

Driver* Driver::createDriver(DriverType type) {
    switch(type) {
    case DriverType::MYSQL:
        return new MySqlDriver();
    case DriverType::SQLITE:
        return new SqliteDriver();
    }
    return nullptr;
}

QString Driver::quote(QVariant value) {
    QSqlField f;
    f.setType(value.type());
    f.setValue(value);
    return driver()->formatValue(f);
}
