/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SQLMODEL_H_
#define _SEQUELJOE_SQLMODEL_H_

#include "tabledata.h"

#include <QAbstractTableModel>
#include <QSet>

class DbConnection;

class SqlModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SqlModel(DbConnection& db, QObject *parent = 0);

    void setQuery(QString q);
    void refresh();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) {
        return false;
    }

    virtual bool insertRows(int row, int count, const QModelIndex &parent);


    virtual bool deleteRows(QSet<int>) {
        return false;
    }

signals:
    void selectFinished();

public slots:

private slots:
    void selectComplete(TableData data);
    void deleteComplete();

protected:
    QString tableName_;


    TableData data_;
    bool isAdding_;
    DbConnection& db_;
    QString query_;
};

#endif // _SEQUELJOE_SQLMODEL_H_
