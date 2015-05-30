#include "driver.h"
#include "roles.h"
#include "foreignkey.h"

#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QSqlField>
#include <QSet>

class SqlDriverList : public QAbstractListModel {
public:
    SqlDriverList() : QAbstractListModel()
    {
        QSqlDatabase db;
        drivers = db.drivers().toSet().intersect({"QSQLITE","QSQLCIPHER","QMYSQL","QPSQL"}).toList();
    }

    int rowCount(const QModelIndex &parent) const {
        return drivers.count();
    }

    QString label(QString driver) const {
        if(driver == "QSQLITE") return "SQLite";
        if(driver == "QSQLCIPHER") return "SQLite/Cipher";
        if(driver == "QMYSQL") return "MySQL/MariaDB";
        if(driver == "QPSQL") return "PostgreSQL";
        return "Unknown";
    }


    bool setData(const QModelIndex &index, const QVariant &value, int role) {
        savedDriver = value.toString();
        return true;
    }

    QVariant data(const QModelIndex &index, int role) const {

        if(index.isValid()) {
            if(role == Qt::DisplayRole)
                return label(drivers.at(index.row()));

            if(role == Qt::EditRole) // hacky :D
                return drivers.at(index.row());

            if(role == DatabaseIsFileRole)
                return drivers.at(index.row()) == "QSQLITE" ||
                        drivers.at(index.row()) == "QSQLCIPHER";

            if(role == DatabaseHasCipherRole)
                return drivers.at(index.row()) == "QSQLCIPHER";

        } else if(role == Qt::EditRole)
            return drivers.indexOf(savedDriver);

        return QVariant();
    }

private:
    QString savedDriver;
    QStringList drivers;
};

class MySqlDriver : public Driver {
public:
    virtual QStringList databases() override {
        QStringList dbnames;
        QSqlQuery q("show databases", *this);
        while(q.next()) {
            dbnames << q.value(0).toString();
        }
        return dbnames;
    }

    virtual void columns(Schema& data, QString table) override {
        data.clear();
        QSqlQuery q{*this};

        q.prepare(
"select c.column_name, c.column_type, c.is_nullable, c.column_key, c.column_default, c.extra, c.column_comment, group_concat(x.constraint_name),group_concat(t.constraint_type),group_concat(x.referenced_table_name), group_concat(x.referenced_column_name) "
"from information_schema.columns as c "
"left join information_schema.key_column_usage as x "
"on c.table_schema = x.table_schema and c.table_name = x.table_name and c.column_name = x.column_name "
"left join information_schema.table_constraints as t "
"on x.constraint_name = t.constraint_name and x.table_schema = t.table_schema and x.table_name = t.table_name "
"where c.table_schema = '"+databaseName()+"' and c.table_name = '"+table+"' group by c.column_name "
"order by c.ordinal_position"
                    );
        q.exec();

        data.columns.reserve(q.size());

        QRegExp typeRegexp("(\\w+)\\(([\\w,]+)\\)\\s*(\\w*)");
        while(q.next()) {
            std::array<QVariant,SCHEMA_NUM_FIELDS> c;
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
            if(!q.value(7).toString().isEmpty()) {
                QStringList constraintNames = q.value(7).toString().split(",");
                QStringList constraintTypes = q.value(8).toString().split(",");
                for(int i = 0; i < constraintNames.length(); ++i) {
                    if(data.constraints.contains(constraintNames[i]))
                        data.constraints[constraintNames[i]].cols.insert(q.value(0).toString());
                    else {
                        ConstraintDetail c;
                        c.type = constraintTypes[i] == "FOREIGN KEY" ? ConstraintDetail::CONSTRAINT_FOREIGNKEY : ConstraintDetail::CONSTRAINT_UNIQUE;
                        if(c.type == ConstraintDetail::CONSTRAINT_FOREIGNKEY) {
                            // todo ON UPDATE, ON DELETE
                            c.fk = ForeignKey{ q.value(0).toString(), q.value(9).toString(), q.value(10).toString() };
                        } else {
                            c.cols.insert(q.value(0).toString());
                        }
                        c.sequence = data.constraints.count();
                        data.constraints[constraintNames[i]] = c;
                    }
                }
                c[SCHEMA_CONSTRAINTS] = constraintNames;
            }
//QVariant v;
//ForeignKey fk;
//fk.table = q.value(7).toString();
//fk.column = q.value(8).toString();
//v.setValue(fk);
//c[SCHEMA_FOREIGNKEY] = v;
            //c[SCHEMA_FOREIGNKEY] = QVariant::fromValue<ForeignKey>({q.value(7).toString(),q.value(8).toString(),q.value(9).toString()});
            data.columns.append(c);
        }
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
                metadata.columnTypes[i] = q.value(7).toString();
                metadata.columnComments[i] = q.value(1).toString();
                metadata.foreignKeys[i] = {q.value(0).toString(), q.value(4).toString(), q.value(5).toString() };
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

    virtual bool open() override {
        QSqlQuery q(*this);
        // yes, it's called twice. somehow doesn't work well otherwise
        QSqlDatabase::open();
        q.exec("SET GLOBAL sql_mode = 'ANSI_QUOTES'");
        setConnectOptions("MYSQL_OPT_RECONNECT=1");
        return QSqlDatabase::open();
    }

    virtual QString createTableQuery(QString table) override {
        return "CREATE TABLE \"" + table + "\" (\"id\" INT UNSIGNED PRIMARY KEY NOT NULL AUTO_INCREMENT)";
    }

};

class SqliteDriver : public Driver {
public:
    virtual void columns(Schema& data, QString table) override {
        data.clear();
        QSqlQuery q{*this};
        q.prepare("PRAGMA table_info(" + table + ")");
        q.exec();
        while(q.next()) {
            std::array<QVariant,SCHEMA_NUM_FIELDS> c;
            c[SCHEMA_NAME] = q.value(1).toString();
            c[SCHEMA_TYPE] = q.value(2).toString().toUpper();
            c[SCHEMA_UNSIGNED] = (q.value(2).toString().contains("unsigned", Qt::CaseInsensitive));
            c[SCHEMA_LENGTH] = 0;
            c[SCHEMA_NULL] = bool(q.value(3).toInt());
            c[SCHEMA_KEY] = "";
            c[SCHEMA_DEFAULT] = q.value(4).toString();
            c[SCHEMA_EXTRA] = "";
            c[SCHEMA_COMMENT] = "";
            data.columns.append(c);
        }
    }

    virtual QStringList databases() override {
        return QStringList{};
    }

    virtual TableMetadata metadata(QString table) override {
        QSqlQuery q{*this};
        TableMetadata metadata;
        // too lazy to implement foreign key support. would probably require regex matching the sql column from sqlite_master

        q.prepare("PRAGMA table_info('" + table + "')");
        q.exec();

        int i = 0;
        while(q.next()) {
            if(q.value(5).toBool())
                metadata.primaryKeyColumn = i;
            i++;
        }
        metadata.resize(i);

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

    virtual QString createTableQuery(QString table) override {
        return "CREATE TABLE \"" + table + "\" (\"id\" INT UNSIGNED PRIMARY KEY NOT NULL AUTO_INCREMENT)";
    }

};

class SqlcipherDriver : public SqliteDriver
{
    virtual bool open() {
        bool ret = QSqlDatabase::open();
        QSqlQuery q(*this);
        return ret && q.exec("PRAGMA key = '" + password() + "'");
    }
};

class PostgresDriver : public Driver {
public:
    virtual QStringList databases() override {
        QStringList dbnames;
        // todo fix
        QSqlQuery q("\\list", *this);
        while(q.next()) {
            dbnames << q.value(0).toString();
        }
        return dbnames;
    }

    virtual void columns(Schema& data, QString table) override {
        data.clear();
        QSqlQuery q{*this};
        q.prepare("select c.column_name, c.data_type, c.is_nullable, c.column_default, u.table_name, u.column_name, u.constraint_name "
                  "from information_schema.columns as c "
                  "left join information_schema.key_column_usage as k "
                  "on c.table_catalog = k.table_catalog and c.table_name = k.table_name and c.column_name = k.column_name "
                  "left join information_schema.table_constraints as t "
                  "on k.constraint_name = t.constraint_name and t.constraint_type = 'FOREIGN KEY' "
                  "left join information_schema.constraint_column_usage as u "
                  "on t.constraint_name = u.constraint_name "
                  "where c.table_catalog = '"+databaseName()+"' and c.table_name = '"+table+"' "
                  "order by c.ordinal_position");
        q.exec();

        data.columns.reserve(q.size());

        QRegExp typeRegexp("(\\w+)\\(([\\w,]+)\\)\\s*(\\w*)");
        while(q.next()) {
            std::array<QVariant,SCHEMA_NUM_FIELDS> c;
            c[SCHEMA_NAME] = q.value(0).toString();
            if(typeRegexp.exactMatch(q.value(1).toString())) {
                c[SCHEMA_TYPE] = typeRegexp.cap(1).toUpper();
                c[SCHEMA_LENGTH] = typeRegexp.cap(2);
            } else
                c[SCHEMA_TYPE] = q.value(1).toString().toUpper(); //e.g. TEXT has no length or unsigned
            c[SCHEMA_NULL] = (q.value(2).toString() == "YES");
            c[SCHEMA_DEFAULT] = q.value(3).toString();
            data.columns.append(c);
        }
    }

    virtual TableMetadata metadata(QString table) override {
        QSqlQuery q{*this};
        TableMetadata metadata;
        q.prepare(
            "select c.column_name, c.data_type, c.column_name = pu.column_name as is_primary, u.table_name, u.column_name, u.constraint_name "
            "from information_schema.columns as c "
            "left join information_schema.table_constraints as p "
            "on c.table_catalog = p.table_catalog and c.table_name = p.table_name and p.constraint_type = 'PRIMARY KEY' "
            "left join information_schema.constraint_column_usage as pu "
            "on p.constraint_name = pu.constraint_name and c.column_name = pu.column_name "
            "left join information_schema.key_column_usage as k "
            "on c.table_catalog = k.table_catalog and c.table_name = k.table_name and c.column_name = k.column_name "
            "left join information_schema.table_constraints as t "
            "on k.constraint_name = t.constraint_name and t.constraint_type = 'FOREIGN KEY' "
            "left join information_schema.constraint_column_usage as u "
            "on t.constraint_name = u.constraint_name "
            "where c.table_catalog = '"+databaseName()+"' and c.table_name = '"+table+"' "
            "order by c.ordinal_position"
        );
        q.exec();

        if(q.first()) {
            int i = 0;
            metadata.resize(q.size());
            do {
                if(q.value(2).toBool())
                    metadata.primaryKeyColumn = i;
                metadata.columnTypes[i] = q.value(1).toString();
                metadata.foreignKeys[i] = {q.value(3).toString(), q.value(4).toString(), q.value(5).toString() };
                i++;

            } while(q.next());
        }
        return metadata;
    }

    virtual QStringList tableNames() override {
        return this->tables();
    }

    virtual QString createTableQuery(QString table) override {
        return "CREATE TABLE \"" + table + "\" (\"id\" SERIAL NOT NULL PRIMARY KEY)";
    }
};

QAbstractListModel* Driver::driverListModel() {
    return new SqlDriverList();
}

Driver* Driver::createDriver(QString type) {
    if(type == "QMYSQL")
        return new MySqlDriver();
    else if(type == "QSQLITE")
        return new SqliteDriver();
    else if(type == "QSQLCIPHER")
        return new SqlcipherDriver();
    else if(type == "QPSQL")
        return new PostgresDriver();
    return nullptr;
}

QString Driver::quote(QVariant value) {
    QSqlField f;
    f.setType(value.type());
    f.setValue(value);
    return driver()->formatValue(f);
}
