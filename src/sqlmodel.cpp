/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "sqlmodel.h"
#include "driver.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlResult>
#include <QStringList>
#include <QSqlError>
#include <QPushButton>

SqlModel::SqlModel(DbConnection &db, QObject *parent) :
    QAbstractItemModel(parent),
    db(db),
    dataSafe(false),
    updatingRow(-1),
    totalRecords(-1),
    rowsFrom(0),
    rowsLimit(0)
{
    content.clear();
    content.columnNames.clear();
}

int SqlModel::columnCount(const QModelIndex &parent) const {
    if(!dataSafe)
        return 0;

    if(parent.isValid())
        return 1;

    return content.columnNames.count();
}

bool SqlModel::columnIsBoolType(int col) const {
    if(!dataSafe)
        return false;

    if(col < metadata.count())
        return metadata.columnTypes.at(col).contains("bool", Qt::CaseInsensitive);

    return false;
}

int SqlModel::rowCount(const QModelIndex &parent) const {
    if(!dataSafe)
        return 0;

    if(parent.isValid())
        return 1;

    return content.count() + (updatingRow == content.count() ? 1 : 0);
}

QVariant SqlModel::data(const QModelIndex &index, int role) const {
    if(role == HeightMultiple) {
        if(dataSafe && index.column() < metadata.count()) {
            QString type = metadata.columnTypes.at(index.column());
            if(type.contains("text", Qt::CaseInsensitive))
                return 4;
            if(type.contains("varchar", Qt::CaseInsensitive))
                return 2;
        }
        return 1;
    }

    if(!dataSafe)
        return QVariant();

    if(!index.isValid())
        return QVariant{};

    if(columnIsBoolType(index.column())) {
        if(role == Qt::CheckStateRole) {
            if(index.row() == updatingRow && !currentRowModifications[index.column()].isNull())
                return currentRowModifications[index.column()].toBool() ? Qt::Checked : Qt::Unchecked;
            if(index.row() < content.count())
                return content.at(index.row()).at(index.column()).toBool() ? Qt::Checked : Qt::Unchecked;
        }
    } else {
        if(index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole) && index.row() < rowCount() && index.column() < columnCount()) {
            QVariant d;
            if(index.row() == updatingRow && currentRowModifications.contains(index.column()))
                d = currentRowModifications[index.column()];
            else if(index.row() < content.count())
                d = content.at(index.row()).at(index.column());

            if(role == Qt::EditRole)
                return d;
            else if(d.type() == QVariant::String)
                return d.toString().replace("\n","");
            else // Qt::DisplayRole
                return d;
        }
    }

    return QVariant();
}

bool SqlModel::insertRows(int row, int count, const QModelIndex &parent) {
    int rows = rowCount();
    beginInsertRows(parent, rows, rows+1);
    updatingRow = rows;
    endInsertRows();
    return true;
}

bool SqlModel::hasChildren(const QModelIndex &parent) const {
    if(!parent.isValid())
        return true;
    if(parent.parent().isValid())
        return false;
    if(dataSafe && parent.column() < metadata.foreignKeys.count() && metadata.foreignKeys[parent.column()].constraint.isNull())
        return true;
    return false;
}

QModelIndex SqlModel::index(int row, int column, const QModelIndex &parent) const {
    if(!dataSafe)
        return QModelIndex{};

    if(parent.isValid()) {
        return createIndex(row, column, quintptr(parent.row()));
    } else if(row >= 0 && column >= 0 && column < columnCount() &&
          ((isAdding() && row <= content.count()) || (!isAdding() && row < content.count()))) {
        return createIndex(row, column, quintptr(-1));
    }

    return QModelIndex{};
}

QModelIndex SqlModel::parent(const QModelIndex &child) const {
    if(!child.isValid())
        return QModelIndex();
    quintptr idx = quintptr(child.internalPointer());
    if(idx == quintptr(-1))
        return QModelIndex();
    return createIndex(idx, 0, quintptr(-1));
}

QVariant SqlModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if(section < content.columnNames.count()) {
            switch(role) {
            case Qt::DisplayRole:
                return content.columnNames.at(section);
            case Qt::ToolTipRole: // move to tablemodel
                return metadata.columnComments.at(section);
            }
        }
    }
    return QVariant();
}

void SqlModel::select() {
    beginResetModel();

    QString query = prepareQuery();
    if(rowsLimit)
    query += " LIMIT " + QString::number(rowsFrom) + ", " + QString::number(rowsLimit);

    QMetaObject::invokeMethod(&db, "queryTableContent", Qt::QueuedConnection, Q_ARG(QString, query), Q_ARG(QObject*, this));
}

void SqlModel::selectComplete(TableData data) {
    content = data;
    dataSafe = true;
    emit selectFinished();
    emit pagesChanged(rowsFrom, rowCount(), totalRecords);
    endResetModel();
}

void SqlModel::firstPage() {
    rowsFrom = 0;
    select();
}

void SqlModel::nextPage() {
    rowsFrom += rowsPerPage();
    select();
}

void SqlModel::prevPage() {
    rowsFrom -= rowsPerPage();
    select();
}

void SqlModel::lastPage() {
    rowsFrom = totalRecords - rowsPerPage();
    select();
}

Qt::ItemFlags SqlModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if(metadata.primaryKeyColumn != -1)
        f |= Qt::ItemIsEditable;
    return f;
}

bool SqlModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(role == ExpandedColumnIndexRole) {
        if(value.toInt() == -1)
            expandedColumns.remove(index.row());
        else
            expandedColumns[index.row()] = value.toInt();
        return true;
    }

    if(role == Qt::CheckStateRole) {
        updatingRow = index.row();
        currentRowModifications[index.column()] = (value.toInt() == Qt::Checked);
        submit();
        return true;

    } else if(role == Qt::EditRole) {
        updatingRow = index.row();
        currentRowModifications[index.column()] = value;
        return true;
    }

    return false;
}

void SqlModel::updateComplete(bool result, int insertId) {
    if(result) {
        if(updatingRow == content.count()) {
            QVector<QVariant> newRow;
            newRow.resize(columnCount());
            if(metadata.primaryKeyColumn != -1)
                currentRowModifications[metadata.primaryKeyColumn] = insertId;

            for(auto it = currentRowModifications.cbegin(); it != currentRowModifications.cend(); it++)
                newRow[it.key()] = it.value();

            content.append(newRow);
            totalRecords++;
        } else {
            for(auto it = currentRowModifications.cbegin(); it != currentRowModifications.cend(); it++)
                content[updatingRow][it.key()] = it.value();
        }
    }
    for(int col: currentRowModifications.keys()) {
        QModelIndex updatedIndex = index(updatingRow, col);
        emit dataChanged(updatedIndex, updatedIndex);
    }
    currentRowModifications.clear();
    updatingRow = -1;
}

bool SqlModel::event(QEvent * e) {
    if(e->type() == QEvent::Type(RefreshEvent))
        return select(), true;
    return QAbstractItemModel::event(e);
}
