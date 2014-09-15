/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "sqlindexmodel.h"
#include "dbconnection.h"

SqlIndexModel::SqlIndexModel(DbConnection &db, QString tableName, QObject *parent) : AbstractSqlModel(db, tableName, parent)
{
}
SqlIndexModel::~SqlIndexModel() {}
void SqlIndexModel::describe(const Filter&) {
    //select();
    beginResetModel();
    QMetaObject::invokeMethod(&db_, "queryTableIndices", Qt::QueuedConnection, Q_ARG(QString, tableName_),
                              Q_ARG(QObject*, this));

}
#include <QDebug>
void SqlIndexModel::describeComplete(Indices data) {
    qDebug() << __PRETTY_FUNCTION__;
    //data_ = data;
    indices_ = data;
    for(Index& i : indices_) for(Index::Member& m : i.members) qDebug() << m.column;
    endResetModel();
}

QModelIndex SqlIndexModel::index(int row, int column, const QModelIndex &parent) const {
//    if(!hasIndex(row, column, parent))
//        return QModelIndex{};
qDebug() << row << column << parent.row();
    if(!parent.isValid())
        return createIndex(row, column, quintptr(-1));
    else { //todo
        qDebug() << "creating child index";
        if(row < indices_.at(parent.row()).members.count())
        return createIndex(row, column, quintptr(parent.row()));
        return QModelIndex{};
}
}

QModelIndex SqlIndexModel::parent(const QModelIndex &child) const {
    if(!child.isValid())
        return QModelIndex();
//qDebug() << __PRETTY_FUNCTION__;
    quintptr idx = quintptr(child.internalPointer());
    if(idx == quintptr(-1))
        return QModelIndex();
    return createIndex(idx, 0, quintptr(-1));
}

int SqlIndexModel::rowCount(const QModelIndex &idx) const {
    if(idx.column() > 0)
        return 0;
    if(idx.parent().isValid())
        return 0;
    if(!idx.isValid())
        return indices_.count();
    qDebug() << idx.internalPointer();
    int i = (quintptr) idx.internalPointer();

    if(i >= indices_.count())
        return 0;//qDebug() << "WTF";

    if(i == -1)
        return indices_.count();
    else //todo
        return indices_.at(i).members.count();
}

int SqlIndexModel::columnCount(const QModelIndex &idx) const {
    return !idx.parent().isValid() ? 3 : 1;
}

QVariant SqlIndexModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole) {
        //if(index.column() == 0)
        if(!index.parent().isValid()) {
            if(index.column() == 0)
                return indices_.at(index.row()).name;
        } else {
            if(index.column() == 1)
                return indices_.at(index.parent().row()).members.at(index.row()).column;
        }

    }

    return QVariant();
}

QVariant SqlIndexModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case 0: return "Name";
        case 1: return "Column";
        case 2: return "Cardinality";
        }
    }
    return QVariant();
}
