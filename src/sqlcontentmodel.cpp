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
QWidget* w;
SqlContentModel::SqlContentModel(DbConnection *db, QString table, QObject *parent) :
    QAbstractTableModel(parent),
    db_(*db),
    tableName_(table),
    primaryKeyIndex_(-1),
    totalRecords_(0),
    rowsFrom_(0),
    rowsLimit_(rowsPerPage()),
    isAdding_(false),
    query_(new QSqlQuery(*db))
{
    describe();
    w = new QPushButton("test");
}

SqlContentModel::~SqlContentModel() {
    delete query_;
}

int SqlContentModel::rowCount(const QModelIndex &parent) const {
    return query_->size() + (isAdding_?1:0);
}

int SqlContentModel::columnCount(const QModelIndex &parent) const {
    return columns_.count();
}

QVariant SqlContentModel::data(const QModelIndex &index, int role) const {
    if(role == FilterColumnRole)
        return whereColumn_;
    if(role == FilterOperationRole)
        return whereOperation_;
    if(role == FilterValueRole)
        return whereValue_;

    if(role == ForeignKeyTableRole)
        return columns_[index.column()].fk_table;
    if(role == ForeignKeyColumnRole)
        return columns_[index.column()].fk_column;

    if(index.row() == query_->size())
        return QVariant(); // during editing only. isAdding_ should be true here
//if(role == Qt::CheckStateRole) return true;
    if(role == WidgetRole) return (qulonglong)w;
    if(index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        query_->seek(index.row());
        return query_->value(index.column());
    }
    return QVariant();
}

QVariant SqlContentModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch(role) {
    case Qt::DisplayRole:
        return columns_.at(section).name;
    case Qt::ToolTipRole:
        return columns_.at(section).comment;
    default:
        return QVariant();
    }
}

void SqlContentModel::describe() {
    // todo this will be different for other SQL drivers

    // primary key required if it should be editable
    QSqlQuery q(db_);
    q.prepare(
        "select c.column_name, c.column_comment, c.column_key = 'PRI' as is_primary, k.referenced_table_name, k.referenced_column_name, t.table_rows "
        "from information_schema.columns as c "
        "inner join information_schema.tables as t "
        "on c.table_schema = t.table_schema and c.table_name = t.table_name "
        "left join information_schema.key_column_usage as k "
        "on c.table_schema = k.table_schema and c.table_name = k.table_name and c.column_name = k.column_name and referenced_column_name is not null "
        "where c.table_schema = '"+db_.databaseName()+"' and c.table_name = '"+tableName_+"'"
    );
    db_.execQuery(q);
    columns_.clear();
    while(q.next()) {
        if(q.value(2).toBool())
            primaryKeyIndex_ = columns_.count();
        columns_.append({q.value(0).toString(), q.value(1).toString(), q.value(3).toString(), q.value(4).toString()});
        totalRecords_ = q.value(5).toInt();
    }
    q.finish();

//    q.prepare("SELECT count(1) FROM " + tableName_);
//    db_.execQuery(q);
//    q.next();
    q.finish();
}

void SqlContentModel::select()
{
    beginResetModel();

    QString where;
    if(!whereValue_.isEmpty()) {
        where = " WHERE `" + whereColumn_ + "` = :value";
    }
    query_->prepare("SELECT * FROM " + tableName_ + where + " LIMIT :top, :count");
    query_->bindValue(":value", whereValue_);
    query_->bindValue(":top", rowsFrom_);
    query_->bindValue(":count", rowsLimit_);
    //query_->exec();
    db_.execQuery(*query_);
    endResetModel();
    emit pagesChanged(rowsFrom_, rowsLimit_, totalRecords_);
}


void SqlContentModel::nextPage() {
    rowsFrom_ += rowsPerPage();
    select();
}

void SqlContentModel::prevPage() {
    rowsFrom_ -= rowsPerPage();
    if(rowsFrom_ < 0)
        rowsFrom_ = 0;
    select();
}

Qt::ItemFlags SqlContentModel::flags(const QModelIndex &index) const {
    return  Qt::ItemIsEnabled | Qt::ItemIsSelectable | (primaryKeyIndex_==-1?Qt::NoItemFlags:Qt::ItemIsEditable);
}


bool SqlContentModel::insertRows(int row, int count, const QModelIndex &parent) {
    beginInsertRows(parent, columns_.count(), columns_.count()+1);
    isAdding_ = true;
    endInsertRows();
    return true;
}

bool SqlContentModel::removeRows(int row, int count, const QModelIndex &parent) {
    QStringList rowIds;
    for(int i = row; i < row+count; ++i)
        rowIds << data(index(i, primaryKeyIndex_)).toString();
    QSqlQuery q(db_);
    q.prepare("DELETE FROM `" + tableName_ + "` WHERE `" + columns_.at(primaryKeyIndex_).name + "` IN (?)");
    q.bindValue(0, rowIds.join(","));
    if(db_.execQuery(q))
        totalRecords_--;
    else
        qDebug() << q.lastError().text();
}

bool SqlContentModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(role == Qt::CheckStateRole)
        qDebug() << "Got check state role!";

    if(role == FilterColumnRole) {
        whereColumn_ = value.toString();
        return true;

    } else if(role == FilterOperationRole) {
        whereOperation_ = value.toString();
        return true;

    } else if(role == FilterValueRole) {
        whereValue_ = value.toString();
        return true;

    } else if(role == Qt::EditRole) {
        if(index.row() == query_->size()) {
            QSqlQuery q(db_);
            q.prepare("INSERT INTO " + tableName_ + "(`" + columns_.at(primaryKeyIndex_).name + "`) VALUES(?)");
            q.bindValue(0, value.toString());
            isAdding_ = false;
            if(db_.execQuery(q)) {
                totalRecords_++;
                return true;
            }else
                qDebug() << q.lastError().text();
        } else {
        QSqlQuery q(db_);
        q.prepare("UPDATE " + tableName_ + " SET `" + columns_.at(index.column()).name + "` = ? WHERE `" + columns_.at(primaryKeyIndex_).name + "` = ?");
        q.bindValue(0, value.toString());
        q.bindValue(1, data(this->index(index.row(), primaryKeyIndex_)).toString());
        if(db_.execQuery(q))
            select();
        else
            qDebug() << q.lastError().text();
        }
    }
    return false;
}

bool SqlContentModel::event(QEvent * e) {
    if(e->type() == RefreshEvent)
        return select(), true;
    return QAbstractTableModel::event(e);
}
