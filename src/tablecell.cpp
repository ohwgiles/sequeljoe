/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tablecell.h"
#include "sqlcontentmodel.h"
#include "tableview.h"
#include "textcelleditor.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QPushButton>
#include <QMouseEvent>
#include <QLineEdit>
#include <QMessageBox>
//QWidget* w;
TableCell::TableCell(SubwidgetFactory& swf, QObject *parent) :
    QStyledItemDelegate(parent),
    subwidgetFactory_(swf)
{
    //w = new QLineEdit(this);
}

QSize TableCell::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    //return QStyledItemDelegate::sizeHint(option, index);
    QSize sz = QStyledItemDelegate::sizeHint(option, index);

    if(0 && index.isValid() && index.parent().isValid() && index.parent().data(ExpandedColumnIndexRole).toInt() == index.column()) {
        //QWidget* w = static_cast<QWidget*>((void*)(index.data(WidgetRole).toULongLong()));
        QWidget* w = subwidgetFactory_.createTableView(index);
        if(w) {
            sz.setHeight(w->sizeHint().height() + sz.height());
        }
    }
    return sz;
}
QWidget* TableCell::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
//    QMessageBox* b = new QMessageBox("title", "text", QMessageBox::Information, int(QMessageBox::Ok),0,0);
//    connect(b, &QMessageBox::accepted, [=](){ closeEditor(b); });
    if(index.data(SqlTypeRole).toString() == "text")
        return new TextCellEditor(parent);
    else
        return QStyledItemDelegate::createEditor(parent, option, index);

}
void TableCell::setEditorData(QWidget *editor, const QModelIndex &index) const {
    TextCellEditor* tce = qobject_cast<TextCellEditor*>(editor);
    if(tce)
        tce->setContent(index.data().toString());
    else
        QStyledItemDelegate::setEditorData(editor, index);
}

void TableCell::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    TextCellEditor* tce = qobject_cast<TextCellEditor*>(editor);
    if(tce)
        model->setData(index, tce->content());
    else
        QStyledItemDelegate::setModelData(editor, model, index);

}
void TableCell::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    TextCellEditor* tce = qobject_cast<TextCellEditor*>(editor);
    if(tce)
        // todo: these are just a guess
        editor->setGeometry(0,0,350,500);
    else
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);

}

#include <QAbstractScrollArea>
void TableCell::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItemV4 opt(option);
    initStyleOption(&opt, index);

    if(index.parent().isValid()) {
opt.rect.setLeft(0);
opt.features &= ~QStyleOptionViewItem::HasDecoration;
        //qDebug() << "parent column: " << index.parent().column();
        if(0 && index.column() == index.parent().data(ExpandedColumnIndexRole).toInt()) {
            //QWidget* w = static_cast<QWidget*>((void*)(index.data(WidgetRole).toULongLong()));
            QWidget* w = qobject_cast<QWidget*>(subwidgetFactory_.createTableView(index));

            //connect(w, SIGNAL())
            //qDebug() << "got widget " << w;
            if(w) {
            //QStyledItemDelegate::paint(painter, option, index);
                //opt.rect.setHeight(opt.rect.height()*2);
            //qDebug() << "painting: " << index.row() << index.column();
            //opt.rect.adjust(0,w->sizeHint().height(), 0, w->sizeHint().height());
            QAbstractScrollArea* area = qobject_cast<QAbstractScrollArea*>(parent());
            Q_ASSERT(area);
            //QSize sz = sizeHint(opt, index);
            QRect pos;
            pos.setLeft(0);
            pos.setRight(area->viewport()->width());
            pos.setTop(opt.rect.top() + sizeHint(option, index.parent()).height());
            pos.setHeight(w->sizeHint().height());
            //opt.rect.setLeft(0);
            //opt.rect.setRight(area->viewport()->width());
            w->setGeometry(pos);
            opt.rect.setHeight(w->sizeHint().height());
            //opt.rect.adjust(0, 0, 0, sz.height());
            QStyledItemDelegate::paint(painter, opt, index);
//w->recalculateRowHeights();
            //if(!w->isVisible()) {
            w->show();




            //}


            }
        }

    } else {
        if(!index.data(ForeignKeyTableRole).toString().isNull()) {
            int indicatorWidth = opt.rect.height() * 2 / 3;
opt.decorationSize = QSize(indicatorWidth,indicatorWidth);
opt.features |= QStyleOptionViewItem::HasDecoration;
            //opt.rect.setLeft(opt.rect.left() + indicatorWidth);

            QStyledItemDelegate::paint(painter, opt, index);
//            opt.rect.setLeft(0);// = expandWidgetRect(opt.rect);
opt.rect.setWidth(indicatorWidth);
//opt.decorationAlignment = Qt::AlignLeft;
            QStyle* s = QApplication::style();
            if(index.sibling(index.row(), 0).data(ExpandedColumnIndexRole).toInt() == index.column()) {
                opt.state = QStyle::State_Open;
                s->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter);
            } else {
                opt.state = QStyle::State_Children;
                s->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter);
            }
        } else
            QStyledItemDelegate::paint(painter, opt, index);


        if(index.data().isNull() && index.data(Qt::CheckStateRole).isNull()) {
            painter->setPen(Qt::lightGray);
            painter->drawText(opt.rect, "null");
        }
    }

}
void TableCell::handleCollapse(const QModelIndex &index) {
    return;
    qDebug() << "collapsing:"<<index.data(ExpandedColumnIndexRole).toInt();
    qDebug() << index.sibling(index.row(), index.data(ExpandedColumnIndexRole).toInt());
    //QWidget* w = static_cast<QWidget*>((void*)(index.child(0, index.data(ExpandedColumnIndexRole).toInt()).data(WidgetRole).toULongLong()));
    QWidget* w = subwidgetFactory_.createTableView(index.child(0, index.data(ExpandedColumnIndexRole).toInt()));
    qDebug() << "2got widget " << w;

    if(w)
        w->hide();
    QAbstractScrollArea* area = qobject_cast<QAbstractScrollArea*>(parent());
    area->update();
}
void TableCell::handleExpand(const QModelIndex& index) {
    QAbstractScrollArea* area = qobject_cast<QAbstractScrollArea*>(parent());
    qDebug() << __PRETTY_FUNCTION__;
    while(area) {
        qDebug() << "updating geometry of " << area;
    area->updateGeometry();
    area = qobject_cast<QAbstractScrollArea*>(area->parentWidget());
    }


//    QWidget* w = qobject_cast<QWidget*>((void*)(index.data(WidgetRole).toULongLong()));
//            if(w) { w->show();w->adjustSize(); }
    //area->repaint();
}

bool TableCell::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if(event->type() == QEvent::MouseButtonPress && !index.data(ForeignKeyTableRole).toString().isNull()) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        QRect r = option.rect.adjusted(0,0,-option.rect.width() + option.rect.height() * 2 /3,0);
        if(r.contains(me->pos())) {
            emit goToForeignEntry(index);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QRect TableCell::expandWidgetRect(QRect r) const { //todo remove
    r.adjust(r.width() - r.height(), 0, 0, 0);
    return r;
}
