#include "sqlschemamodel.h"
#include <QColor>
SqlSchemaModel::SqlSchemaModel(QSqlDatabase* db, QString tableName, QObject *parent) :
    QSqlQueryModel(parent)
{
    setQuery("DESCRIBE " + tableName, *db);
}
int SqlSchemaModel::columnCount(const QModelIndex &parent) const
{
    return QSqlQueryModel::columnCount(parent) + 1;
}
QVariant SqlSchemaModel::data(const QModelIndex &item, int role) const {
    if(item.column() > 1)
        return QSqlQueryModel::data(item.model()->index(item.row(), item.column()-1, item.parent()), role);
    else
        return QSqlQueryModel::data(item,role);
}
QVariant SqlSchemaModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(section > 1)
        return QSqlQueryModel::headerData(section - 1, orientation, role);
    else
        return QSqlQueryModel::headerData(section, orientation, role);

}
