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
    QString table;
    QString column;
    QString constraint;
    bool isNull() const { return column.isNull(); }
};

Q_DECLARE_METATYPE(ForeignKey)

#endif // _SEQUELJOE_FOREIGNKEY_H_
