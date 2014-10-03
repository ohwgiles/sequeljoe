/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_TABLEMODEL_H
#define _SEQUELJOE_TABLEMODEL_H

#include "sqlmodel.h"

class TableModel : public SqlModel {
    Q_OBJECT
public:
    explicit TableModel(DbConnection& db, QString query, QObject *parent = 0);
    explicit TableModel(TableModel& model, QString query, QObject* parent = 0) : TableModel(model.db, query, parent) {}
    virtual ~TableModel() {}

    virtual void describe(const Filter &where = Filter{});
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    void setFilter(Filter& f) { where = f; select();}

protected slots:
    bool submit() override;
    void revert() override;

protected:
    virtual QString prepareQuery() const override;
    virtual bool deleteRows(QSet<int>) override;

private slots:
    void describeComplete(TableMetadata metadata);

private:
    QString tableName;
    Filter where;
};

#endif // _SEQUELJOE_TABLEMODEL_H
