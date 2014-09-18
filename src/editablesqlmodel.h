/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_EDITABLESQLMODEL_H_
#define _SEQUELJOE_EDITABLESQLMODEL_H_

#include "abstractsqlmodel.h"
#include "tabledata.h"

class EditableSqlModel : public AbstractSqlModel {
public:
    EditableSqlModel(DbConnection& db, QString tableName, QObject* parent = 0);

    virtual bool insertRows(int row, int count, const QModelIndex &parent);
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

protected:
    bool dataSafe_;

    TableData data_;
    bool isAdding_;
};

#endif // _SEQUELJOE_EDITABLESQLMODEL_H_
