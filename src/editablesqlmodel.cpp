/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */

#include "editablesqlmodel.h"

EditableSqlModel::EditableSqlModel(DbConnection& db, QString tableName, QObject* parent) :
    AbstractSqlModel(db, tableName, parent),
    isAdding_(false)
{

}

bool EditableSqlModel::insertRows(int row, int count, const QModelIndex &parent) {
    int rows = rowCount();
    beginInsertRows(parent, rows, rows+1);
    isAdding_ = true;
    endInsertRows();
    return true;
}
#include <QDebug>
QVariant EditableSqlModel::data(const QModelIndex &index, int role) const {
    if(index.row() == data_.count())
        return QVariant(); // during editing only. isAdding_ should be true here

    //qDebug() << "role:"<<role<<"index:"<<index<<"datacount:"<<data_.count();
    if(index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        if(index.row() < data_.count() && index.column() < data_.at(index.row()).count())
        return data_.at(index.row()).at(index.column());
    }

    return QVariant();
}
