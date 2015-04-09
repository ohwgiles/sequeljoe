#include "constraintitemdelegate.h"


void ConstraintItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItemV4 opt(option);
    initStyleOption(&opt, index);
    painter->save();
    painter->setPen(index.data(Qt::TextColorRole).value<QColor>());
    painter->drawText(opt.rect, "\u2666");
    painter->restore();
    opt.rect.adjust(15,0,15,0);
    painter->drawText(opt.rect, opt.text);
}

QSize ConstraintItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QStyleOptionViewItemV4 opt(option);
    opt.features |= QStyleOptionViewItemV4::HasCheckIndicator;
    return QStyledItemDelegate::sizeHint(opt, index);
}

bool ConstraintItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if(event->type() == QEvent::MouseButtonPress) {
        emit constraintActivated(index);
        return true;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
