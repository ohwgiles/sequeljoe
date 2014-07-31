/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_TABLECELL_H_
#define _SEQUELJOE_TABLECELL_H_

#include <QItemDelegate>

class TableCell : public QItemDelegate
{
    Q_OBJECT
public:
    explicit TableCell(QObject *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // _SEQUELJOE_TABLECELL_H_
