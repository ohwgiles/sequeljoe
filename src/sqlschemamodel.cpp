#include "sqlschemamodel.h"
#include <QColor>
#include <QSqlQuery>
#include <QVector>

#include <QDebug>

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

SqlSchemaModel::SqlSchemaModel(QSqlDatabase* db, QString tableName, QObject *parent) :
    QAbstractTableModel(parent),
    tableName_(tableName),
    db_(*db)
{
    // todo vary per db type
    query_ = new QSqlQuery("SHOW FULL COLUMNS FROM " + tableName, *db);
    QRegExp typeRegexp("(\\w+)\\((\\w+)\\)\\s*(\\w*)");
    while(query_->next()) {
        SqlColumn c;
        c.name = query_->value(0).toString();
        qDebug() << query_->value(1).toString();
        if(typeRegexp.exactMatch(query_->value(1).toString())) {
            c.type = typeRegexp.cap(1).toUpper();
            c.length = typeRegexp.cap(2);
            c.is_unsigned = (typeRegexp.cap(3) == "unsigned");
        } else
            c.type = query_->value(1).toString(); //e.g. TEXT has no length or unsigned
        c.allow_null = (query_->value(3).toString() == "YES");
        c.key = query_->value(4).toString();
        c.default_value = query_->value(5).toString();
        c.extra = query_->value(6).toString();
        c.collation = query_->value(2).toString();
        c.comment = query_->value(8).toString();
        columns_.append(c);
    }
}
int SqlSchemaModel::columnCount(const QModelIndex &parent) const
{
    return SCHEMA_NUM_FIELDS;
}

int SqlSchemaModel::rowCount(const QModelIndex& parent) const {
    return columns_.count();
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
    if(item.isValid() && item.row() < columns_.count()) {
        const SqlColumn& c = columns_.at(item.row());
        if(role == Qt::DisplayRole || role == Qt::EditRole) {
            switch(item.column()) {
            case SCHEMA_NAME:
                return c.name;
            case SCHEMA_TYPE:
                return c.type;
            case SCHEMA_LENGTH:
                return c.length;
            case SCHEMA_KEY:
                return c.key;
            case SCHEMA_DEFAULT:
                return c.default_value;
            case SCHEMA_EXTRA:
                return c.extra;
            case SCHEMA_COLLATION:
                return c.collation;
            case SCHEMA_COMMENT:
                return c.comment;
            default: break;
            }
        } else if(role == Qt::CheckStateRole) {
            switch(item.column()) {
            case SCHEMA_UNSIGNED:
                return c.is_unsigned ? Qt::Checked : Qt::Unchecked;
            case SCHEMA_NULL:
                return c.allow_null ? Qt::Checked : Qt::Unchecked;
            default: break;
            }
        }
    }
    return QVariant();
}

QString SqlSchemaModel::getColumnChangeQuery(QString column, const SqlColumn& def) const {
    // todo proper escaping?
    return QString("ALTER TABLE ") +
            "`" + tableName_ + "` CHANGE " +
            "`" + column + "` " +
            "`" + def.name + "` " +
            def.type + "(" + def.length + ")" +
            (def.is_unsigned ? " UNSIGNED" : "") +
            (!def.allow_null ? " NOT NULL" : "") +
            (!def.default_value.isNull() ? " DEFAULT " + def.default_value : "") +
            (!def.extra.isNull() ? def.extra : "") +
            (!def.collation.isNull() ? " COLLATE '" + def.collation + "'" : "") +
            (!def.comment.isNull() ? " COMMENT '" + def.comment + "'" : "");
}
#include <QSqlError>
bool SqlSchemaModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    qDebug() << "setting " << index.column() << "," << index.row() << " to " << value;
    if(index.isValid() && role == Qt::EditRole) {
        // make a copy
        SqlColumn c = columns_.at(index.row());
        QString columnName = c.name;
        QSqlQuery q(db_);
        bool runQuery = false;
        switch(index.column()) {
        case SCHEMA_NAME:
            c.name = value.toString();
            runQuery = true;
            break;
        case SCHEMA_TYPE:
            c.type = value.toString();
            runQuery = true;
            break;
        case SCHEMA_LENGTH:
            c.length = value.toString();
            runQuery = true;
            break;
        case SCHEMA_DEFAULT:
            c.default_value = value.toString();
            runQuery = true;
            break;
        case SCHEMA_COLLATION:
            c.collation = value.toString();
            runQuery = true;
            break;
        case SCHEMA_COMMENT:
            c.comment = value.toString();
            runQuery = true;
            break;

        default:
            break;

        }
        if(runQuery) {
            QString query = getColumnChangeQuery(columnName, c);
            qDebug() << "Executing: " << query;
            if(q.exec(query)) {
                columns_[index.row()] = c;
                return true;
            } else {
                qDebug() << "error: " << q.lastError();
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
