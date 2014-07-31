/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tablecell.h"

#include <QPainter>

TableCell::TableCell(QObject *parent) :
    QItemDelegate(parent)
{
}

void TableCell::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QItemDelegate::paint(painter, option, index);
    if(index.data().isNull() && index.data(Qt::CheckStateRole).isNull()) {
        painter->setPen(Qt::lightGray);
        painter->drawText(option.rect, "null");
    }
}
