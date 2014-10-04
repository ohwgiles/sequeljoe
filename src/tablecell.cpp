/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tablecell.h"
#include "sqlmodel.h"
#include "tableview.h"
#include "textcelleditor.h"

#include <QPainter>
#include <QApplication>
#include <QPushButton>
#include <QMouseEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QAbstractScrollArea>

TableCell::TableCell(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QWidget* TableCell::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
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

void TableCell::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItemV4 opt(option);
    initStyleOption(&opt, index);

    if(index.parent().isValid()) {
        opt.rect.setLeft(0);
        opt.features &= ~QStyleOptionViewItem::HasDecoration;
    } else {
        if(!index.data(ForeignKeyTableRole).toString().isNull()) {
            int indicatorWidth = opt.rect.height() * 2 / 3;
            opt.decorationSize = QSize(indicatorWidth,indicatorWidth);
            opt.features |= QStyleOptionViewItem::HasDecoration;
            QStyledItemDelegate::paint(painter, opt, index);
            opt.rect.setWidth(indicatorWidth);
            QStyle* s = QApplication::style();
            if(index.sibling(index.row(), 0).data(ExpandedColumnIndexRole).toInt() == index.column()) {
                opt.state = QStyle::State_Children | QStyle::State_Open;
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

bool TableCell::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if(event->type() == QEvent::MouseButtonPress && !index.data(ForeignKeyTableRole).toString().isNull()) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        QRect r = option.rect.adjusted(0,0,-option.rect.width() + option.rect.height() * 2 /3,0);
        if(r.contains(me->pos())) {
            emit requestForeignKey(index);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

