/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_QUERYMODEL_H_
#define _SEQUELJOE_QUERYMODEL_H_

#include "tabledata.h"

#include <QAbstractTableModel>
#include <QSet>

class DbConnection;

#include "abstractsqlmodel.h"

class QueryModel : public AbstractSqlModel
{
    Q_OBJECT
public:
    explicit QueryModel(DbConnection& db, QObject *parent = 0);

    void setQuery(QString q);
    void refresh();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) {
        return false;
    }

    virtual bool hasChildren(const QModelIndex &parent) const override { if(!parent.isValid()) return true;return false; }
    virtual void describe(const Filter& filter = Filter{}) {}

    virtual bool deleteRows(QSet<int>) {
        return false;
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::Horizontal) const override;

signals:
    void selectFinished();

public slots:

private slots:
    void selectComplete(TableData data);
    void deleteComplete();

protected:
    TableData data_;
    QString query_;
};

#endif // _SEQUELJOE_QUERYMODEL_H_
