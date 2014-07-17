#include "sqlschemamodel.h"
#include <QColor>
#include <QSqlQuery>
#include <QVector>

#include <QDebug>

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
        }
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
    return 10;
}

int SqlSchemaModel::rowCount(const QModelIndex& parent) const {
    return columns_.count();
}

Qt::ItemFlags SqlSchemaModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if(index.isValid()) {
        switch(index.column()) {
        case 0: flags |= Qt::ItemIsEditable; break;
        case 3: flags |= Qt::ItemIsUserCheckable; break;
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
            case 0:
                return c.name;
            case 1:
                return c.type;
            case 2:
                return c.length;
            case 5:
                return c.key;
            case 6:
                return c.default_value;
            case 7:
                return c.extra;
            case 8:
                return c.collation;
            case 9:
                return c.comment;
            default: break;
            }
        } else if(role == Qt::CheckStateRole) {
            switch(item.column()) {
            case 3:
                return c.is_unsigned ? Qt::Checked : Qt::Unchecked;
            case 4:
                return c.allow_null ? Qt::Checked : Qt::Unchecked;
            default: break;
            }
        }
    }
    return QVariant();
}

bool SqlSchemaModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    qDebug() << "setting " << index.column() << "," << index.row() << " to " << value;
    if(index.isValid() && role == Qt::EditRole) {
        SqlColumn& c = columns_[index.row()];
        QSqlQuery q(db_);
        switch(index.column()) {
        case 0:
            if(q.exec("ALTER TABLE `" + tableName_ + "` CHANGE `" +
                        c.name + "` `" + value.toString() + "` " + c.type + "(" + c.length + ")")) {
                c.name = value.toString();

            return true;}
        default:
            break;

        }
    }
    return false;
}

QVariant SqlSchemaModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role == Qt::DisplayRole) {
        switch(section) {
        case 0: return "Name";
        case 1: return "Type";
        case 2: return "Length";
        case 3: return "Unsigned";
        case 4: return "Allow Null";
        case 5: return "Key";
        case 6: return "Default";
        case 7: return "Extra";
        case 8: return "Collation";
        case 9: return "Comment";
        default: break;
        }
    }
    return QVariant();
}
