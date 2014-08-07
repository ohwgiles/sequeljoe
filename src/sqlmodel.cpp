/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "sqlmodel.h"

#include "dbconnection.h"
#include <QDebug>
SqlModel::SqlModel(DbConnection &db, QObject *parent) :
    QAbstractTableModel(parent),
    isAdding_(false),
    db_(db)
{
}


int SqlModel::rowCount(const QModelIndex &parent) const {
    return data_.size() + (isAdding_?1:0);
}
int SqlModel::columnCount(const QModelIndex &parent) const {
    return data_.size() ? data_.at(0).count() : 0;
}

QVariant SqlModel::data(const QModelIndex &index, int role) const {
    if(index.row() == data_.count())
        return QVariant(); // during editing only. isAdding_ should be true here

    if(index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        return data_.at(index.row()).at(index.column());
    }

    return QVariant();
}

bool SqlModel::insertRows(int row, int count, const QModelIndex &parent) {
    int rows = rowCount();
    beginInsertRows(parent, rows, rows+1);
    isAdding_ = true;
    endInsertRows();
    return true;
}


void SqlModel::deleteComplete() {
    refresh();
}

void SqlModel::setQuery(QString q) {
    query_ = q;
}

void SqlModel::refresh() {
    beginResetModel();
    QMetaObject::invokeMethod(&db_, "queryTableContent", Qt::QueuedConnection, Q_ARG(QString, query_), Q_ARG(QObject*, this));
}

void SqlModel::selectComplete(TableData data) {
    qDebug() << data;
    data_ = data;
    endResetModel();
}
