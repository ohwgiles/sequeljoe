#include "sqlcontentmodel.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlResult>
SqlContentModel::SqlContentModel(QSqlDatabase *db, QString table, QObject *parent) :
    QAbstractTableModel(parent),
    db_(*db),
    tableName_(table),
    totalRecords_(0),
    rowsFrom_(0),
    rowsLimit_(rowsPerPage()),
    query_(new QSqlQuery(*db))
{
    describe();
    //select();
}
int SqlContentModel::rowCount(const QModelIndex &parent) const {
    return query_->size();
}
int SqlContentModel::columnCount(const QModelIndex &parent) const {
    return columns_.count();
}
QVariant SqlContentModel::data(const QModelIndex &index, int role) const {
    if(index.isValid() && role == Qt::DisplayRole) {
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
    QSqlQuery q("SHOW COLUMNS FROM " + tableName_, db_);
    columns_.clear();
    while(q.next()) {
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
