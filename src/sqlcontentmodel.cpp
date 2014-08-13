/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "sqlcontentmodel.h"
#include "dbconnection.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlResult>
#include <QDebug>
#include <QStringList>
#include <QSqlError>
#include <QPushButton>

SqlContentModel::SqlContentModel(DbConnection &db, QString table, QObject *parent) :
    SqlModel(db, parent),
    db_(db),
    updatingIndex_(-1),
    totalRecords_(0),
    rowsFrom_(0),
    rowsLimit_(rowsPerPage())
{
    columns_.clear();
    tableName_ =table;
    primaryKeyIndex_ =-1;
}

SqlContentModel::~SqlContentModel() {
}

int SqlContentModel::columnCount(const QModelIndex &parent) const {
    return columns_.count();
}

QVariant SqlContentModel::data(const QModelIndex &index, int role) const {
    if(role == FilterColumnRole)
        return where_.column;
    if(role == FilterOperationRole)
        return where_.operation;
    if(role == FilterValueRole)
        return where_.value;

    if(role == ForeignKeyTableRole)
        return columns_[index.column()].fk_table;
    if(role == ForeignKeyColumnRole)
        return columns_[index.column()].fk_column;

    return SqlModel::data(index, role);
}

QVariant SqlContentModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(section < columns_.count()) {
        switch(role) {
        case Qt::DisplayRole:
            return columns_.at(section).name;
        case Qt::ToolTipRole:
            return columns_.at(section).comment;
        }
    }
    return QVariant();
}

void SqlContentModel::describe(const Filter& where) {
    beginResetModel();
    where_ = where;
    QMetaObject::invokeMethod(&db_, "queryTableMetadata", Qt::QueuedConnection, Q_ARG(QString, tableName_), Q_ARG(QObject*, this));
}

void SqlContentModel::describeComplete(QVector<ColumnHeader> columns, int totalRecords, int primaryKeyIndex) {
    // mutex lock necessary?
    columns_ = columns;
    totalRecords_ = totalRecords;
    primaryKeyIndex_ = primaryKeyIndex;
    modifiedRow_.resize(columns_.count());
    select();
}
#include <QSqlField>
#include <QSqlDriver>
void SqlContentModel::select() {
    beginResetModel();

    QString query = "SELECT * FROM " + tableName_;
    if(!where_.value.isEmpty()) {
        QSqlField f;
        f.setValue(where_.value);
        query += " WHERE `" + where_.column + "` " + where_.operation + " " + db_.driver()->formatValue(f);
    }
    query += " LIMIT " + QString::number(rowsFrom_) + ", " + QString::number(rowsLimit_);

    QMetaObject::invokeMethod(&db_, "queryTableContent", Qt::QueuedConnection, Q_ARG(QString, query), Q_ARG(QObject*, this));
}

void SqlContentModel::selectComplete(QVector<QVector<QVariant>> data) {
    data_ = data;
    endResetModel();
    emit pagesChanged(rowsFrom_, rowsLimit_, totalRecords_);
}

void SqlContentModel::nextPage() {
    rowsFrom_ += rowsPerPage();
    select();
}

void SqlContentModel::prevPage() {
    rowsFrom_ -= rowsPerPage();
    select();
}

Qt::ItemFlags SqlContentModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if(primaryKeyIndex_ != -1 && updatingIndex_ == -1)
        f |= Qt::ItemIsEditable;
    return f;
}


bool SqlContentModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(role == Qt::CheckStateRole)
        qDebug() << "Got check state role!";

    if(role == FilterColumnRole) {
        where_.column = value.toString();
        return true;

    } else if(role == FilterOperationRole) {
        where_.operation = value.toString();
        return true;

    } else if(role == FilterValueRole) {
        where_.value = value.toString();
        return true;

    } else if(role == Qt::EditRole) {
        updatingIndex_ = index.row();
        updatingColumn_ = index.column();

        if(index.row() == data_.size()) {
            QSqlQuery q(db_);
            //todo escape
            QString query("INSERT INTO " + tableName_ + "(`" + columns_.at(primaryKeyIndex_).name + "`) VALUES('" + value.toString() + "')");
            QMetaObject::invokeMethod(&db_, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this));
return true;
//            q.prepare();
//            q.bindValue(0, value.toString());
//            isAdding_ = false;
//            if(db_.execQuery(q)) {
//                totalRecords_++;
//                return true;
//            }else
//                qDebug() << q.lastError().text();
        } else {
            modifiedRow_ = data_[index.row()];
            modifiedRow_[index.column()] = value.toString();
            QString query("UPDATE " + tableName_ + " SET `" + columns_.at(index.column()).name + "` = '" + value.toString() + "' WHERE `" +
                          columns_.at(primaryKeyIndex_).name +"` = '" + data(this->index(index.row(), primaryKeyIndex_)).toString() + "'");
            QMetaObject::invokeMethod(&db_, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this));
            return true;
        }
    }
    return false;
}

void SqlContentModel::updateComplete(bool result, int insertId) {
    if(result) {
        // INSERT

        isAdding_ = false;
        if(updatingIndex_ == data_.count()) {
            //beginInsertRows(QModelIndex(), updatingIndex_, updatingIndex_+1);
            modifiedRow_[primaryKeyIndex_] = insertId;
            data_.append(modifiedRow_);
            totalRecords_++;
            //endInsertRows();

        }
        // UPDATE
        else {
            data_[updatingIndex_] = modifiedRow_;
        }
        modifiedRow_.clear();
        modifiedRow_.resize(columns_.count());
        QModelIndex updatedIndex = index(updatingIndex_, updatingColumn_);
        emit dataChanged(updatedIndex, updatedIndex);
        updatingIndex_ = -1;

    }
}
bool SqlContentModel::deleteRows(QSet<int> rows) {
    QStringList rowIds;
    for(int i : rows) {
        rowIds << data(index(i, primaryKeyIndex_)).toString();
        data_.remove(i);
    }
    QString query("DELETE FROM `" + tableName_ + "` WHERE `" + columns_.at(primaryKeyIndex_).name + "` IN ("+rowIds.join(",")+")");

    QMetaObject::invokeMethod(&db_, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this), Q_ARG(const char*,"deleteComplete"));
    return true;
}

bool SqlContentModel::event(QEvent * e) {
    if(e->type() == QEvent::Type(RefreshEvent))
        return select(), true;
    return QAbstractTableModel::event(e);
}
