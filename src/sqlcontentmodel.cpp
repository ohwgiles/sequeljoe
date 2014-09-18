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
    EditableSqlModel(db, table, parent),
    updatingIndex_(-1),
    totalRecords_(0),
    rowsFrom_(0),
    rowsLimit_(rowsPerPage())
{
}

SqlContentModel::~SqlContentModel() {
}

int SqlContentModel::columnCount(const QModelIndex &parent) const {
    if(!dataSafe_)
        return 0;

    if(parent.isValid())
        return 1;
    return metadata_.count();
}
int SqlContentModel::rowCount(const QModelIndex &parent) const {
    if(!dataSafe_)
        return 0;

    if(parent.isValid()) return 1;
    return data_.count() + (isAdding_?1:0);
}
#include "tableview.h"
QVariant SqlContentModel::data(const QModelIndex &index, int role) const {
    if(!dataSafe_)
        return QVariant();

    if(!index.isValid()) return QVariant();
    if(role == ExpandedColumnIndexRole) {
        if(!expandedColumns_.contains(index.row()))
            return quintptr(-1);
        return expandedColumns_.value(index.row());
    }

    if(role == SqlTypeRole)
        return metadata_.columnTypes[index.column()];

    if(role == FilterColumnRole)
        return where_.column;
    if(role == FilterOperationRole)
        return where_.operation;
    if(role == FilterValueRole)
        return where_.value;

    if(role == ForeignKeyTableRole)
        return metadata_.foreignKeyTables[index.column()];
    if(role == ForeignKeyColumnRole)
        return metadata_.foreignKeyColumns[index.column()];

    return EditableSqlModel::data(index, role);
}
bool SqlContentModel::hasChildren(const QModelIndex &parent) const {
    if(!parent.isValid())
        return true;
    if(parent.parent().isValid())
        return false;
    if(metadata_.foreignKeyTables[parent.column()].isNull())
        return true;
    return true; ///???
    return false;
}
QModelIndex SqlContentModel::index(int row, int column, const QModelIndex &parent) const {
//    if(!hasIndex(row, column, parent))
//        return QModelIndex{};
//qDebug() << row << column << parent.row();
    if(!parent.isValid())
        return createIndex(row, column, quintptr(-1));
    else { //todo
        //qDebug() << "creating child index";
//        if(row < indices_.at(parent.row()).members.count())
        return createIndex(row, column, quintptr(parent.row()));
        return QModelIndex{};
}
}

QModelIndex SqlContentModel::parent(const QModelIndex &child) const {
    if(!child.isValid())
        return QModelIndex();
//qDebug() << __PRETTY_FUNCTION__;
    quintptr idx = quintptr(child.internalPointer());
    if(idx == quintptr(-1))
        return QModelIndex();
    return createIndex(idx, 0, quintptr(-1));
}

QVariant SqlContentModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if(section < metadata_.columnNames.count()) {
            switch(role) {
            case Qt::DisplayRole:
                return metadata_.columnNames.at(section);
            case Qt::ToolTipRole:
                return metadata_.columnComments.at(section);
            }
        }
    }
    return QVariant();
}

void SqlContentModel::describe(const Filter& where) {
    dataSafe_ = false;
    beginResetModel();
    where_ = where;
    QMetaObject::invokeMethod(&db_, "queryTableMetadata", Qt::QueuedConnection, Q_ARG(QString, tableName_), Q_ARG(QObject*, this));
}

void SqlContentModel::describeComplete(TableMetadata metadata) {
    metadata_ = metadata;
    //modifiedRow_.resize(metadata.count());
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
        query += " WHERE `" + where_.column + "` " + where_.operation + " '" + db_.driver()->formatValue(f) + "'";
    }
    query += " LIMIT " + QString::number(rowsFrom_) + ", " + QString::number(rowsLimit_);

    QMetaObject::invokeMethod(&db_, "queryTableContent", Qt::QueuedConnection, Q_ARG(QString, query), Q_ARG(QObject*, this));
}

void SqlContentModel::selectComplete(TableData data) {
    data_ = data;
    endResetModel();
    dataSafe_ = true;
    emit selectFinished();
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
    if(metadata_.primaryKeyColumn != -1 && updatingIndex_ == -1)
        f |= Qt::ItemIsEditable;
    return f;
}


bool SqlContentModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(role == ExpandedColumnIndexRole) {
        if(value.toInt() == -1)
            expandedColumns_.remove(index.row());
        else
            expandedColumns_[index.row()] = value.toInt();
        return true;
    }

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
            QString query("INSERT INTO " + tableName_ + "(`" + metadata_.columnNames.at(metadata_.primaryKeyColumn) + "`) VALUES('" + value.toString() + "')");
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
            QString query("UPDATE " + tableName_ + " SET `" + metadata_.columnNames.at(index.column()) + "` = '" + value.toString() + "' WHERE `" +
                          metadata_.columnNames.at(metadata_.primaryKeyColumn) +"` = '" + data(this->index(index.row(), metadata_.primaryKeyColumn)).toString() + "'");
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
            modifiedRow_[metadata_.primaryKeyColumn] = insertId;
            data_.append(modifiedRow_);
            totalRecords_++;
            //endInsertRows();

        }
        // UPDATE
        else {
            data_[updatingIndex_] = modifiedRow_;
        }
        modifiedRow_.clear();
        modifiedRow_.resize(metadata_.count());
        QModelIndex updatedIndex = index(updatingIndex_, updatingColumn_);
        emit dataChanged(updatedIndex, updatedIndex);
        updatingIndex_ = -1;

    }
}
bool SqlContentModel::deleteRows(QSet<int> rows) {
    QStringList rowIds;
    for(int i : rows) {
        rowIds << data(index(i, metadata_.primaryKeyColumn)).toString();
        data_.remove(i);
    }
    QString query("DELETE FROM `" + tableName_ + "` WHERE `" + metadata_.columnNames.at(metadata_.primaryKeyColumn) + "` IN ("+rowIds.join(",")+")");

    QMetaObject::invokeMethod(&db_, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this), Q_ARG(const char*,"deleteComplete"));
    return true;
}

bool SqlContentModel::event(QEvent * e) {
    if(e->type() == QEvent::Type(RefreshEvent))
        return select(), true;
    return AbstractSqlModel::event(e);
}
