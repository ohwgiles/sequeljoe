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
    setQuery("SELECT * FROM " + table);
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
    select();
}

QString TableModel::prepareQuery() const {
    if(!where.value.isEmpty()) {
        QSqlField f;
        f.setValue(where.value);
        return query + " WHERE `" + where.column + "` " + where.operation + " '" + db.sqlDriver()->driver()->formatValue(f) + "'";
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

    if(role == SqlTypeRole) // used by TableCell to know to use the popup editor
        return metadata.columnTypes[index.column()];

    if(role == FilterColumnRole)
        return where.column;
    if(role == FilterOperationRole)
        return where.operation;
    if(role == FilterValueRole)
        return where.value;

    if(role == ForeignKeyTableRole)
        return metadata.foreignKeyTables[index.column()];
    if(role == ForeignKeyColumnRole)
        return metadata.foreignKeyColumns[index.column()];

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
                QSqlField f;
                f.setValue(it.value());
                values << db.sqlDriver()->driver()->formatValue(f);
            }
            QString query = "INSERT INTO `" + tableName + "` (`" + columns.join("`,`") + "`) VALUES('" + values.join("','") + "')";
            QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this));
        } else {
            QStringList updates;
            for(auto it = currentRowModifications.cbegin(); it != currentRowModifications.cend(); ++it) {
                QSqlField f;
                f.setValue(it.value());
                updates << "`" + content.columnNames[it.key()] + "` = '" + db.sqlDriver()->driver()->formatValue(f) + "'";
            }
            QString query = "UPDATE `" + tableName + "` SET " + updates.join(", ") + " WHERE `" +
                    metadata.columnNames.at(metadata.primaryKeyColumn) +"` = '" + data(index(updatingRow, metadata.primaryKeyColumn)).toString() + "'";
            QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this));
        }
        return true;
    }
    return false;
}

void TableModel::revert() {
    select();
}

bool TableModel::deleteRows(QSet<int> rows) {
    QStringList rowIds;
    for(int i : rows) {
        rowIds << data(index(i, metadata.primaryKeyColumn)).toString();
        content.remove(i);
    }
    QString query("DELETE FROM `" + tableName + "` WHERE `" + metadata.columnNames.at(metadata.primaryKeyColumn) + "` IN ("+rowIds.join(",")+")");

    QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this), Q_ARG(const char*,"deleteComplete"));
    return true;
}

