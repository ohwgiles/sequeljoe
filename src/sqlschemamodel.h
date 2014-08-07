/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SQLSCHEMAMODEL_H_
#define _SEQUELJOE_SQLSCHEMAMODEL_H_

#include "sqlmodel.h"

class DbConnection;

class QSqlQuery;

typedef QVector<QVariant> SqlColumn;

class SqlSchemaModel : public SqlModel
{
    Q_OBJECT
public:
    explicit SqlSchemaModel(DbConnection &db, QString tableName, QObject *parent = 0);
    virtual ~SqlSchemaModel();

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool deleteRows(QSet<int>);

    void describe();
private slots:
    void describeComplete(QVector<QVector<QVariant>> data);
private:
    QString getColumnChangeQuery(QString column, const SqlColumn& def) const;

    //bool isAdding_;
    QSqlQuery* query_;
    //QVector<SqlColumn> columns_;
    QString tableName_;
    DbConnection& db_;
};

#endif // _SEQUELJOE_SQLSCHEMAMODEL_H_
