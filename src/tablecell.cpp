#include "tablecell.h"
#include <QPainter>
TableCell::TableCell(QObject *parent) :
    QItemDelegate(parent)
{
}

void TableCell::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if(index.data().isNull()) {
        painter->setPen(Qt::lightGray);
        painter->drawText(option.rect, "null");
    } else
        return QItemDelegate::paint(painter, option, index);
}
