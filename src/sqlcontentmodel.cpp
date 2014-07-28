#include "sqlcontentmodel.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlResult>
#include <QDebug>
#include <QStringList>
SqlContentModel::SqlContentModel(QSqlDatabase *db, QString table, QObject *parent) :
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
    //select();
}
int SqlContentModel::rowCount(const QModelIndex &parent) const {
    return query_->size() + (isAdding_?1:0);
}
int SqlContentModel::columnCount(const QModelIndex &parent) const {
    return columns_.count();
}
QVariant SqlContentModel::data(const QModelIndex &index, int role) const {
    if(index.row() == query_->size())
        return QVariant(); // during editing only. isAdding_ should be true here

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
void SqlContentModel::describe()
{
    // primary key required if it should be editable
    QSqlQuery key("SHOW KEYS FROM "+tableName_+" WHERE Key_name = 'PRIMARY'", db_);
    key.next();
    QString primaryKey = key.value(4).toString();

    QSqlQuery q("SHOW COLUMNS FROM " + tableName_, db_);
    columns_.clear();
    while(q.next()) {
        if(q.value(0).toString() == primaryKey)
            primaryKeyIndex_ = columns_.count();
        columns_.append({q.value(0).toString(), q.value(1).toString()});
    }

    QSqlQuery count("SELECT count(*) FROM " + tableName_, db_);
    count.next();
    totalRecords_ = count.value(0).toInt();


}

void SqlContentModel::select()
{
    beginResetModel();
    query_->prepare("SELECT * FROM " + tableName_ + " LIMIT ?,?");
    query_->bindValue(0, rowsFrom_);
    query_->bindValue(1, rowsLimit_);
    query_->exec();
    endResetModel();
    emit pagesChanged(rowsFrom_, rowsLimit_, totalRecords_);
}

void SqlContentModel::nextPage() {
    rowsFrom_ += rowsPerPage();
    select();
}

void SqlContentModel::prevPage() {
    rowsFrom_ -= rowsPerPage();
    if(rowsFrom_ < 0) rowsFrom_ = 0;
    select();
}

Qt::ItemFlags SqlContentModel::flags(const QModelIndex &index) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | (primaryKeyIndex_==-1?Qt::NoItemFlags:Qt::ItemIsEditable);
}


bool SqlContentModel::insertRows(int row, int count, const QModelIndex &parent) {
    beginInsertRows(parent, columns_.count(), columns_.count()+1);
    isAdding_ = true;
    endInsertRows();
    return true;
}
#include <QSqlError>
bool SqlContentModel::removeRows(int row, int count, const QModelIndex &parent) {
    QStringList rowIds;
    for(int i = row; i < row+count; ++i)
        rowIds << data(index(i, primaryKeyIndex_)).toString();
    qDebug() << "row: " << row << ", count: " << count;
    QSqlQuery q(db_);
    q.prepare("DELETE FROM `" + tableName_ + "` WHERE `" + columns_.at(primaryKeyIndex_).name + "` IN (?)");
    q.bindValue(0, rowIds.join(","));
    if(q.exec())
    {totalRecords_--;}//select();
    else
        qDebug() << q.lastError().text();
//    beginRemoveRows(parent, row, row);
//    columns_.remove(row);
//    endRemoveRows();
}
bool SqlContentModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    qDebug() << "setting " << index.column() << "," << index.row() << " to " << value;
    if(!index.isValid()) return false;
    if(role == Qt::EditRole) {
        if(index.row() == query_->size()) {
            QSqlQuery q(db_);
            q.prepare("INSERT INTO " + tableName_ + "(`" + columns_.at(primaryKeyIndex_).name + "`) VALUES(?)");
            q.bindValue(0, value.toString());
            isAdding_ = false;
            if(q.exec()) {
                totalRecords_++;
                return true;
            }else
                qDebug() << q.lastError().text();
        } else {
        QSqlQuery q(db_);
        q.prepare("UPDATE " + tableName_ + " SET `" + columns_.at(index.column()).name + "` = ? WHERE `" + columns_.at(primaryKeyIndex_).name + "` = ?");
        q.bindValue(0, value.toString());
        q.bindValue(1, data(this->index(index.row(), primaryKeyIndex_)).toString());
        if(q.exec())
            select();
        else
            qDebug() << q.lastError().text();
        }
    }
    return false;
}
