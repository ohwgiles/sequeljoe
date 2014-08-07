/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_TABLEDATA_H_
#define _SEQUELJOE_TABLEDATA_H_

#include <QVector>
#include <QVariant>

typedef QVector<QVector<QVariant>> TableData;

struct ColumnHeader {
    QString name;
    QString comment;
    QString fk_table;
    QString fk_column;
};


#endif // _SEQUELJOE_TABLEDATA_H_
