/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SQLINDEXMODEL_H_
#define _SEQUELJOE_SQLINDEXMODEL_H_

#include "abstractsqlmodel.h"
#include "tabledata.h"
class SqlIndexModel : public AbstractSqlModel
{
    Q_OBJECT
public:
    explicit SqlIndexModel(DbConnection &db, QString tableName, QObject *parent = 0);
    virtual ~SqlIndexModel();

    virtual void describe(const Filter& filter = Filter{}) override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override;

    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

    virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    //bool hasChildren(const QModelIndex &parent) const override { return true; }
private slots:
    void describeComplete(Indices data);

signals:

public slots:

protected:
    Indices indices_;
};

#endif // _SEQUELJOE_SQLINDEXMODEL_H_
