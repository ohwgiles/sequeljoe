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
#include "foreignkey.h"

struct Index {
    QString name;
    struct Member {
        QString column;
        int sequence;
        bool unique;
        int cardinality;
    };
    QVector<Member> members;
};

typedef QVector<Index> Indices;

struct TableData : public QVector<QVector<QVariant>> {
//    QVector<QString> columnNames;
};

struct TableMetadata {
    void resize(int nColumns) {
        columnTypes.resize(nColumns);
        columnComments.resize(nColumns);
        foreignKeys.resize(nColumns);
        size_ = nColumns;
    }
    int count() const { return size_; }
    int primaryKeyColumn = -1;
    int numRows = -1;
    QVector<QString> columnTypes;
    QVector<QString> columnComments;
    QVector<ForeignKey> foreignKeys;
private:
    int size_ = 0;
};

struct Filter {
    QString column;
    QString operation;
    QString value;
};


#endif // _SEQUELJOE_TABLEDATA_H_
