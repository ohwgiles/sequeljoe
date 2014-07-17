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
