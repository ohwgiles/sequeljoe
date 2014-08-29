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
#include <QLineEdit>
//QWidget* w;
TableCell::TableCell(QObject *parent) :
    QStyledItemDelegate(parent)
{
    //w = new QLineEdit(this);
}

QSize TableCell::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize sz = QStyledItemDelegate::sizeHint(option, index);

    if(index.isValid() && index.parent().isValid() && index.parent().data(ExpandedColumnIndexRole).toInt() == index.column()) {
        QWidget* w = static_cast<QWidget*>((void*)(index.data(WidgetRole).toULongLong()));
    if(w) {
        sz.setHeight(sz.height() + w->sizeHint().height());
    }
    }
    return sz;
}
#include <QAbstractScrollArea>
void TableCell::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItemV4 opt(option);
    initStyleOption(&opt, index);

    if(index.parent().isValid()) {
        qDebug() << "parent column: " << index.parent().column();
        if(index.column() == index.parent().data(ExpandedColumnIndexRole).toInt()) {
            QWidget* w = static_cast<QWidget*>((void*)(index.data(WidgetRole).toULongLong()));
            qDebug() << "got widget " << w;
            if(w) {
            //QStyledItemDelegate::paint(painter, option, index);
                //opt.rect.setHeight(opt.rect.height()*2);
            qDebug() << "painting: " << index.row() << index.column();
            //opt.rect.adjust(0,w->sizeHint().height(), 0, w->sizeHint().height());
            QAbstractScrollArea* area = qobject_cast<QAbstractScrollArea*>(parent());
            Q_ASSERT(area);
            QSize sz = QStyledItemDelegate::sizeHint(opt, index);
            opt.rect.adjust(0, sz.height(), 0, sz.height());
            opt.rect.setLeft(0);
            opt.rect.setRight(area->viewport()->width());
            w->setGeometry(opt.rect);
            w->show();
            }
        }

    } else {

        if(!index.data(ForeignKeyTableRole).toString().isNull()) {
            opt.rect = expandWidgetRect(opt.rect);

            QStyle* s = QApplication::style();
            if(index.sibling(index.row(), 0).data(ExpandedColumnIndexRole).toInt() == index.column()) {
                opt.state = QStyle::State_Open;
                s->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter);
            } else {
                opt.state = QStyle::State_Children;
                s->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter);
            }
            opt.rect.setLeft(opt.rect.left() + s->);
        }
        QStyledItemDelegate::paint(painter, opt, index);

        if(index.data().isNull() && index.data(Qt::CheckStateRole).isNull()) {
            painter->setPen(Qt::lightGray);
            painter->drawText(opt.rect, "null");
        }
    }

}
void TableCell::handleCollapse(const QModelIndex &index) {
    qDebug() << "collapsing:"<<index.data(ExpandedColumnIndexRole).toInt();
    qDebug() << index.sibling(index.row(), index.data(ExpandedColumnIndexRole).toInt());
    QWidget* w = static_cast<QWidget*>((void*)(index.child(0, index.data(ExpandedColumnIndexRole).toInt()).data(WidgetRole).toULongLong()));
    qDebug() << "2got widget " << w;

    if(w)
        w->hide();
    QAbstractScrollArea* area = qobject_cast<QAbstractScrollArea*>(parent());
    area->update();
}
void TableCell::handleExpand(const QModelIndex& index) {
    QAbstractScrollArea* area = qobject_cast<QAbstractScrollArea*>(parent());
    area->update();

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
