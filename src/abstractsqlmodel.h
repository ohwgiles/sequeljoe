/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_ABSTRACTSQLMODEL_H_
#define _SEQUELJOE_ABSTRACTSQLMODEL_H_

#include <QAbstractItemModel>
#include <QSet>
class DbConnection;
struct Filter {
    QString column;
    QString operation;
    QString value;
};
class AbstractSqlModel : public QAbstractItemModel {
public:
    AbstractSqlModel(DbConnection& db, QString tableName, QObject* parent = 0) :
        QAbstractItemModel(parent),
        db_(db),
        tableName_(tableName)
    {}
    virtual void refresh() {} // todo pure?

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override { return createIndex(row, column); }
    virtual QModelIndex parent(const QModelIndex &child = QModelIndex{}) const { return QModelIndex{}; }

    virtual void describe(const Filter& filter = Filter{}) = 0;
    virtual bool deleteRows(QSet<int>) {return false;}

    DbConnection& db() const { return db_; }

protected:

    DbConnection& db_;
    QString tableName_;
};

#endif // _SEQUELJOE_ABSTRACTSQLMODEL_H_
