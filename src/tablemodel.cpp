/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tablemodel.h"
#include "driver.h"

#include <QSqlDriver>
#include <QSqlField>

TableModel::TableModel(DbConnection &db, QString table, QObject *parent) :
    SqlModel(db, parent),
    tableName(table),
    where(Filter{})
{
    setQuery("SELECT * FROM `" + table + "`");
     //todo move all row limiting into this class?
    rowsLimit = rowsPerPage();
}

void TableModel::describe(const Filter& f) {
    dataSafe = false;
    beginResetModel();
    where = f;
    QMetaObject::invokeMethod(&db, "queryTableMetadata", Qt::QueuedConnection, Q_ARG(QString, tableName), Q_ARG(QObject*, this));
}

void TableModel::describeComplete(TableMetadata metadata) {
    this->metadata = metadata;
    totalRecords = metadata.numRows;
    select();
}

QString TableModel::prepareQuery() const {
    if(!where.value.isEmpty()) {
        return query + " WHERE `" + where.column + "` " + where.operation + " " + db.sqlDriver()->quote(where.value);
    } else
        return query;
}

QVariant TableModel::data(const QModelIndex &index, int role) const {
    if(!dataSafe)
        return QVariant();

    if(!index.isValid() || index.column() >= columnCount()) return QVariant();
    if(role == ExpandedColumnIndexRole) {
        if(!expandedColumns.contains(index.row()))
            return quintptr(-1);
        return expandedColumns.value(index.row());
    }

    if(role == EditorTypeRole) // used by TableCell to know to use the popup editor
        return metadata.columnTypes[index.column()].toLower().contains("text") ? SJCellEditLongText : SJCellEditDefault;

    if(role == FilterColumnRole)
        return where.column;
    if(role == FilterOperationRole)
        return where.operation;
    if(role == FilterValueRole)
        return where.value;

    if(role == ForeignKeyRole)
        return QVariant::fromValue<ForeignKey>(metadata.foreignKeys[index.column()]);

    return SqlModel::data(index, role);
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(role == FilterColumnRole) {
        where.column = value.toString();
        return true;
    } else if(role == FilterOperationRole) {
        where.operation = value.toString();
        return true;
    } else if(role == FilterValueRole) {
        where.value = value.toString();
        return true;
    } else
        return SqlModel::setData(index, value, role);
}

bool TableModel::submit() {
    if(updatingRow != -1 && currentRowModifications.count() > 0) {
        if(updatingRow == content.size()) {
            QStringList columns;
            QStringList values;
            for(auto it = currentRowModifications.cbegin(); it != currentRowModifications.cend(); ++it) {
                columns << content.columnNames[it.key()];
                values << db.sqlDriver()->quote(it.value());
            }
            QString query = "INSERT INTO `" + tableName + "` (`" + columns.join("`,`") + "`) VALUES(" + values.join(",") + ")";
            QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this));
        } else {
            QStringList updates;
            for(auto it = currentRowModifications.cbegin(); it != currentRowModifications.cend(); ++it) {
                updates << "`" + content.columnNames[it.key()] + "` = " + db.sqlDriver()->quote(it.value());
            }
            QString query = "UPDATE `" + tableName + "` SET " + updates.join(", ") + " WHERE `" +
                    metadata.columnNames.at(metadata.primaryKeyColumn) +"` = " + db.sqlDriver()->quote(data(index(updatingRow, metadata.primaryKeyColumn), Qt::EditRole));
            QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this));
        }
        return true;
    }
    return false;
}

void TableModel::revert() {
    select();
}
#include <QDebug>
bool TableModel::deleteRows(QSet<int> rows) {
    // if we have a primary key, we can delete just by this id, which efficiently allows
    // us to delete multiple rows in a single query
    if(metadata.primaryKeyColumn > -1) {
        QStringList rowIds;
        for(int i : rows) {
            rowIds << db.sqlDriver()->quote(data(index(i, metadata.primaryKeyColumn)).toString());
            content.remove(i);
        }
        QString query("DELETE FROM `" + tableName + "` WHERE `" + metadata.columnNames.at(metadata.primaryKeyColumn) + "` IN ("+rowIds.join(",")+")");

        QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this), Q_ARG(const char*,"deleteComplete"));
    } else {
        // otherwise we have to compare every column
        QString columnNames = "`" + QStringList(metadata.columnNames.toList()).join("`,`") + "`";
        for(int i : rows) {
            QStringList values;
            for(int j = 0; j < metadata.count(); ++j)
                values << db.sqlDriver()->quote(data(index(i,j)).toString());
            QString query("DELETE FROM `" + tableName + "` WHERE (" + columnNames + ") = (" + values.join(",") + ")");
            content.remove(i);
            QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this), Q_ARG(const char*,"deleteComplete"));
        }

    }
    return true;
}

