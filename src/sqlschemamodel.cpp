/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "sqlschemamodel.h"
#include "dbconnection.h"
#include "notify.h"

#include <QColor>
#include <QSqlQuery>
#include <QVector>
#include <QDebug>
#include <QSqlError>

enum {
    SCHEMA_NAME = 0,
    SCHEMA_TYPE,
    SCHEMA_LENGTH,
    SCHEMA_UNSIGNED,
    SCHEMA_NULL,
    SCHEMA_KEY,
    SCHEMA_DEFAULT,
    SCHEMA_EXTRA,
    SCHEMA_COLLATION,
    SCHEMA_COMMENT,

    SCHEMA_NUM_FIELDS
};

SqlSchemaModel::SqlSchemaModel(DbConnection *db, QString tableName, QObject *parent) :
    QAbstractTableModel(parent),
    tableName_(tableName),
    db_(*db)
{
    isAdding_ = false;
    // todo vary per db type
    query_ = new QSqlQuery(*db);
    query_->prepare("SHOW FULL COLUMNS FROM " + tableName);
    db_.execQuery(*query_);
    QRegExp typeRegexp("(\\w+)\\(([\\w,]+)\\)\\s*(\\w*)");
    while(query_->next()) {
        SqlColumn c;
        c.resize(SCHEMA_NUM_FIELDS);
        c[SCHEMA_NAME] = query_->value(0).toString();
        if(typeRegexp.exactMatch(query_->value(1).toString())) {
            c[SCHEMA_TYPE] = typeRegexp.cap(1).toUpper();
            c[SCHEMA_LENGTH] = typeRegexp.cap(2);
            c[SCHEMA_UNSIGNED] = (typeRegexp.cap(3) == "unsigned");
        } else
            c[SCHEMA_TYPE] = query_->value(1).toString().toUpper(); //e.g. TEXT has no length or unsigned
        c[SCHEMA_NULL] = (query_->value(3).toString() == "YES");
        c[SCHEMA_KEY] = query_->value(4).toString();
        c[SCHEMA_DEFAULT] = query_->value(5).toString();
        c[SCHEMA_EXTRA] = query_->value(6).toString();
        c[SCHEMA_COLLATION] = query_->value(2).toString();
        c[SCHEMA_COMMENT] = query_->value(8).toString();
        columns_.append(c);
    }
}

SqlSchemaModel::~SqlSchemaModel() {
    delete query_;
}

int SqlSchemaModel::columnCount(const QModelIndex &parent) const {
    return SCHEMA_NUM_FIELDS;
}

int SqlSchemaModel::rowCount(const QModelIndex& parent) const {
    return columns_.count() + (isAdding_ ? 1 : 0);
}

Qt::ItemFlags SqlSchemaModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if(index.isValid()) {
        switch(index.column()) {
        case SCHEMA_NAME:
        case SCHEMA_TYPE:
        case SCHEMA_LENGTH:
        case SCHEMA_DEFAULT:
        case SCHEMA_EXTRA:
        case SCHEMA_COLLATION:
        case SCHEMA_COMMENT:
            flags |= Qt::ItemIsEditable;
            break;
        case SCHEMA_UNSIGNED:
        case SCHEMA_NULL:
            flags |= Qt::ItemIsUserCheckable;
            break;
        default: break;
        }
    }
    return flags;
}

QVariant SqlSchemaModel::data(const QModelIndex &item, int role) const {
    if(!item.isValid()) return QVariant();
    if(item.row() < columns_.count()) {
        const SqlColumn& c = columns_.at(item.row());
        if(role == Qt::DisplayRole || role == Qt::EditRole) {
            switch(item.column()) {
            case SCHEMA_NAME:
            case SCHEMA_TYPE:
            case SCHEMA_LENGTH:
            case SCHEMA_KEY:
            case SCHEMA_DEFAULT:
            case SCHEMA_EXTRA:
            case SCHEMA_COLLATION:
            case SCHEMA_COMMENT:
                return c[item.column()];
            default: break;
            }
        } else if(role == Qt::CheckStateRole) {
            switch(item.column()) {
            case SCHEMA_UNSIGNED:
            case SCHEMA_NULL:
                return c[item.column()].toBool() ? Qt::Checked : Qt::Unchecked;
            default: break;
            }
        }
    } else if(item.row() == columns_.count()) {
    }
    return QVariant();
}

QString SqlSchemaModel::getColumnChangeQuery(QString column, const SqlColumn& def) const {
    // todo proper escaping?
    return QString("ALTER TABLE ") +
            "`" + tableName_ + "` CHANGE " +
            "`" + column + "` " +
            "`" + def[SCHEMA_NAME].toString() + "` " +
            def[SCHEMA_TYPE].toString() +
            (!def[SCHEMA_LENGTH].isNull() ? "(" + def[SCHEMA_LENGTH].toString() + ")" : "") +
            (def[SCHEMA_UNSIGNED].toBool() ? " UNSIGNED" : "") +
            (!def[SCHEMA_NULL].toBool() ? " NOT NULL" : "") +
            (!def[SCHEMA_DEFAULT].isNull() ? " DEFAULT " + def[SCHEMA_DEFAULT].toString() : "") +
            (!def[SCHEMA_EXTRA].isNull() ? def[SCHEMA_EXTRA].toString() : "") +
            (!def[SCHEMA_COLLATION].isNull() ? " COLLATE '" + def[SCHEMA_COLLATION].toString() + "'" : "") +
            (!def[SCHEMA_COMMENT].isNull() ? " COMMENT '" + def[SCHEMA_COMMENT].toString() + "'" : "");
}

bool SqlSchemaModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(!index.isValid()) return false;
    if(role == Qt::EditRole) {
        // make a copy
        QSqlQuery q(db_);
        if(index.row() == columns_.count()) {
            if(index.column() == SCHEMA_NAME) {
                q.prepare("ALTER TABLE `" + tableName_ + "` ADD COLUMN `" + value.toString() + "` TEXT");
                if(db_.execQuery(q)) {
                    SqlColumn c;
                    c.resize(SCHEMA_NUM_FIELDS);
                    c[SCHEMA_NAME] = value.toString();
                    columns_.append(c);

                } else {
                    notify->send("Error creating column", q.lastError().text().toLocal8Bit().constData());
                }
                isAdding_ = false;
            } else {
                qDebug() << "shouldn't happen";
            }

        } else {
            SqlColumn c = columns_.at(index.row());
            QString columnName = c[SCHEMA_NAME].toString();

            QString v = value.toString();
            bool runQuery = false;
            switch(index.column()) {
            case SCHEMA_TYPE:
                v = v.toUpper();
                // fall through
            case SCHEMA_NAME:
            case SCHEMA_LENGTH:
            case SCHEMA_DEFAULT:
            case SCHEMA_COLLATION:
            case SCHEMA_COMMENT:
                c[index.column()] = value.toString();
                //qDebug() << "Executing: " << query;
                q.prepare(getColumnChangeQuery(columnName, c));
                if(db_.execQuery(q)) {
                    columns_[index.row()] = c;
                    return true;
                } else {
                    qDebug() << "error: " << q.lastError();
                }
                break;

            default:
                break;

            }
        }
    }
    return false;
}

QVariant SqlSchemaModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role == Qt::DisplayRole) {
        switch(section) {
        case SCHEMA_NAME: return "Name";
        case SCHEMA_TYPE: return "Type";
        case SCHEMA_LENGTH: return "Length";
        case SCHEMA_UNSIGNED: return "Unsigned";
        case SCHEMA_NULL: return "Allow Null";
        case SCHEMA_KEY: return "Key";
        case SCHEMA_DEFAULT: return "Default";
        case SCHEMA_EXTRA: return "Extra";
        case SCHEMA_COLLATION: return "Collation";
        case SCHEMA_COMMENT: return "Comment";
        default: break;
        }
    }
    return QVariant();
}

bool SqlSchemaModel::insertRows(int row, int count, const QModelIndex &parent) {
    beginInsertRows(parent, columns_.count(), columns_.count()+1);
    isAdding_ = true;
    endInsertRows();
    return true;
}

bool SqlSchemaModel::removeRows(int row, int count, const QModelIndex &parent) {
    if(count != 1) return false;
    QString columnName = columns_.at(row).at(SCHEMA_NAME).toString();

    QSqlQuery q(db_);
    q.prepare("ALTER TABLE `" + tableName_ + "` DROP COLUMN `" + columnName + "`");
    if(db_.execQuery(q)) {
        beginRemoveRows(parent, row, row);
        columns_.remove(row);
        endRemoveRows();
        return true;
    } else {
        qDebug() << "could not remove row: " << q.lastError().text();
    }
    return false;
}
