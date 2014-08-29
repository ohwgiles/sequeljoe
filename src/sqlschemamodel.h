/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SQLSCHEMAMODEL_H_
#define _SEQUELJOE_SQLSCHEMAMODEL_H_

#include "abstractsqlmodel.h"
#include "tabledata.h"

class DbConnection;

class QSqlQuery;

typedef QVector<QVariant> SqlColumn;

class SqlSchemaModel : public AbstractSqlModel
{
    Q_OBJECT
public:
    explicit SqlSchemaModel(DbConnection &db, QString tableName, QObject *parent = 0);
    virtual ~SqlSchemaModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override { return createIndex(row, column); }
    QModelIndex parent(const QModelIndex &child = QModelIndex{}) const { return QModelIndex{}; }
    int rowCount(const QModelIndex &parent = QModelIndex{}) const {
        return data_.size() + (isAdding_?1:0);
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool deleteRows(QSet<int>);

    void describe(const Filter& filter = Filter{});
private slots:
    void describeComplete(TableData data);
private:
    QString getColumnChangeQuery(QString column, const SqlColumn& def) const;

    TableData data_;
    bool isAdding_;

    //QSqlQuery* query_;
    //QVector<SqlColumn> columns_;
};

#endif // _SEQUELJOE_SQLSCHEMAMODEL_H_
