/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tablecell.h"
#include "sqlcontentmodel.h"

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QPushButton>
#include <QMouseEvent>

TableCell::TableCell(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void TableCell::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    QStyleOptionViewItemV4 opt(option);
    QStyledItemDelegate::paint(painter, opt, index);
    if(index.data().isNull() && index.data(Qt::CheckStateRole).isNull()) {
        painter->setPen(Qt::lightGray);
        painter->drawText(option.rect, "null");
    }
    if(!index.data(ForeignKeyTableRole).toString().isNull()) {
        QStyleOptionViewItemV4 opt(option);
        opt.state = QStyle::State_Enabled;
        opt.rect = expandWidgetRect(opt.rect);

        QStyle* s = QApplication::style();
        s->drawPrimitive(QStyle::PE_IndicatorArrowRight, &opt, painter);
    }
}

bool TableCell::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if(event->type() == QEvent::MouseButtonPress && !index.data(ForeignKeyTableRole).toString().isNull()) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        QRect r = expandWidgetRect(option.rect);
        if(r.contains(me->pos())) {
            emit goToForeignEntry(index);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QRect TableCell::expandWidgetRect(QRect r) const {
    r.adjust(r.width() - r.height(), 0, 0, 0);
    return r;
}
