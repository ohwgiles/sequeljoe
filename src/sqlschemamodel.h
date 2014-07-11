#ifndef SQLSCHEMAMODEL_H
#define SQLSCHEMAMODEL_H

#include <QSqlQueryModel>

class SqlSchemaModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit SqlSchemaModel(QSqlDatabase* db, QString tableName, QObject *parent = 0);
    virtual int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &item, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
signals:

public slots:

};

#endif // SQLSCHEMAMODEL_H
