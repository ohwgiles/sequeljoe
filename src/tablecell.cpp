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
#include "foreignkeyeditor.h"

#include <QPainter>
#include <QApplication>
#include <QPushButton>
#include <QMouseEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QAbstractScrollArea>

TableCell::TableCell(QObject *parent) :
    QStyledItemDelegate(parent),
    firstColumnIsHeader(false)
{
}

QWidget* TableCell::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    switch(index.data(EditorTypeRole).toInt()) {
    case SJCellEditLongText:
        return new TextCellEditor(parent);
    case SJCellEditForeignKey:
        // todo fix this horrible hack
        return new ForeignKeyEditor(qobject_cast<SqlModel*>(qobject_cast<TableView*>(this->parent())->model())->driver(), parent);
    default:
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void TableCell::setEditorData(QWidget *editor, const QModelIndex &index) const {
    if(TextCellEditor* tce = qobject_cast<TextCellEditor*>(editor))
        tce->setContent(index.data(Qt::EditRole).toString());
    else if(ForeignKeyEditor* fke = qobject_cast<ForeignKeyEditor*>(editor))
        fke->setForeignKey(index.data(Qt::EditRole));
    else
        QStyledItemDelegate::setEditorData(editor, index);
}

void TableCell::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    if(TextCellEditor* tce = qobject_cast<TextCellEditor*>(editor))
        model->setData(index, tce->content());
    else if(ForeignKeyEditor* fke = qobject_cast<ForeignKeyEditor*>(editor))
        model->setData(index, fke->foreignKey());
    else
        QStyledItemDelegate::setModelData(editor, model, index);

}

void TableCell::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QRect g = qobject_cast<QWidget*>(parent())->window()->geometry();
    if(qobject_cast<TextCellEditor*>(editor))
        editor->setGeometry(g.x()+g.width()/8, g.y()+g.height()/8,g.width()*3/4,g.height()*3/4);
    else if(qobject_cast<ForeignKeyEditor*>(editor))
        editor->setGeometry(g.x()+(g.width()-editor->width())/2, g.y()+(g.height()-editor->height())/2,editor->width(),editor->height());
    else
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);

}

void TableCell::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItemV4 opt(option);
    initStyleOption(&opt, index);

    if(index.parent().isValid()) {
        opt.rect.setLeft(0);
        opt.features &= ~QStyleOptionViewItem::HasDecoration;
    } else if(firstColumnIsHeader && index.column() == 0) {
        QStyleOptionHeader hopt;
        hopt.rect = opt.rect;
        hopt.state = QStyle::State_None;
        hopt.orientation = Qt::Vertical;
        hopt.textAlignment = Qt::AlignVCenter;
        hopt.text = index.data().toString();
        if (option.widget->isEnabled())
            hopt.state |= QStyle::State_Enabled;
        hopt.state |= QStyle::State_Active;
        QApplication::style()->drawControl(QStyle::CE_Header, &hopt, painter);
    } else {
        if(!firstColumnIsHeader && !index.data(ForeignKeyRole).value<ForeignKey>().isNull()) {
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
        } else {
            if(index.data().isNull() && index.data(Qt::CheckStateRole).isNull()) {
                opt.palette.setColor(QPalette::Text, Qt::lightGray);
                opt.text = "null";
            }
            QStyledItemDelegate::paint(painter, opt, index);
        }

    }
}

QSize TableCell::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QSize sz = QStyledItemDelegate::sizeHint(option, index);
    return firstColumnIsHeader ? QSize(sz.width(), sz.height() * index.data(HeightMultiple).toInt()) : sz;
}

bool TableCell::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if(event->type() == QEvent::MouseButtonPress && !index.data(ForeignKeyRole).value<ForeignKey>().isNull()) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        QRect r = option.rect.adjusted(0,0,-option.rect.width() + option.rect.height() * 2 /3,0);
        if(r.contains(me->pos())) {
            emit requestForeignKey(index);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

