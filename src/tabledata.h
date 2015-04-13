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
#include <QSet>

struct ConstraintDetail {
    enum {
        CONSTRAINT_FOREIGNKEY,
        CONSTRAINT_UNIQUE
    } type;
    // used for determining legend colour. Untidy, but much simpler and
    // safer than determining from position in Map
    int sequence;
    // following may by empty, depending on type
    ForeignKey fk;
    QSet<QString> cols;
};
struct Constraint {
    QString name;
    ConstraintDetail detail;
};
struct ConstraintMap : public QMap<QString, ConstraintDetail> {
    static Constraint constraint(ConstraintMap::const_iterator it) {
        return { it.key(), it.value() };
    }
};

Q_DECLARE_METATYPE(Constraint)

enum {
    SCHEMA_NAME = 0,
    SCHEMA_TYPE,
    SCHEMA_LENGTH,
    SCHEMA_UNSIGNED,
    SCHEMA_NULL,
    SCHEMA_KEY,
    SCHEMA_DEFAULT,
    SCHEMA_EXTRA,
    SCHEMA_CONSTRAINTS,
    SCHEMA_COMMENT,

    SCHEMA_NUM_FIELDS
};

struct Schema {
    QVector<std::array<QVariant,SCHEMA_NUM_FIELDS>> columns;
    ConstraintMap constraints;
    void clear() {
        columns.clear();
        constraints.clear();
    }
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
