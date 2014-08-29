/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "querymodel.h"

#include "dbconnection.h"
#include <QDebug>
QueryModel::QueryModel(DbConnection &db, QObject *parent) :
    AbstractSqlModel(db, QString{}, parent)
{
}


int QueryModel::rowCount(const QModelIndex &parent) const {
    return data_.count();
}
int QueryModel::columnCount(const QModelIndex &parent) const {
    return data_.columnNames.count();
}

QVariant QueryModel::data(const QModelIndex &index, int role) const {
    if(index.row() == data_.count())
        return QVariant(); // during editing only. isAdding_ should be true here

    if(index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        return data_.at(index.row()).at(index.column());
    }

    return QVariant();
}

QVariant QueryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return data_.columnNames.at(section);
    }
    return section;
}


void QueryModel::deleteComplete() {
    refresh();
}

void QueryModel::setQuery(QString q) {
    query_ = q;
}

void QueryModel::refresh() {
    beginResetModel();
    QMetaObject::invokeMethod(&db_, "queryTableContent", Qt::QueuedConnection, Q_ARG(QString, query_), Q_ARG(QObject*, this));
}

// todo put column names in TableData?
void QueryModel::selectComplete(TableData data) {
    data_ = data;
    endResetModel();
}
