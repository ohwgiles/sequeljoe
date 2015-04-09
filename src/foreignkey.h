/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_FOREIGNKEY_H_
#define _SEQUELJOE_FOREIGNKEY_H_

#include <QString>

struct ForeignKey {
    QString column;
    QString refTable;
    QString refColumn;
    //QString constraint;
    QString onDelete;
    QString onUpdate;
    bool isNull() const { return refColumn.isNull(); }
};

Q_DECLARE_METATYPE(ForeignKey)

#endif // _SEQUELJOE_FOREIGNKEY_H_
