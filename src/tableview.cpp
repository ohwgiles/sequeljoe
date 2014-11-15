/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tableview.h"

#include "tablecell.h"
#include "tablemodel.h"
#include "loadingoverlay.h"

#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QLayout>
#include <QResizeEvent>
#include <QLineEdit>
#include <QLabel>

TableView::TableView(QWidget *parent) :
    QTreeView(parent)
{
    setContentsMargins(0,0,0,0);
    setAlternatingRowColors(true);
    setRootIsDecorated(false);
    setIndentation(0);

    TableCell* tc = new TableCell(this);
    connect(tc, SIGNAL(requestForeignKey(QModelIndex)), this, SLOT(toggleForeignTable(QModelIndex)));
    setItemDelegate(tc);

    // context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(openMenu(QPoint)));
    contextMenu = new QMenu(this);
    setNullAction = new QAction("Set to NULL", contextMenu);
    connect(setNullAction, SIGNAL(triggered()), this, SLOT(handleSetNull()));
    deleteRowAction = new QAction("Delete row", contextMenu);
    connect(deleteRowAction, SIGNAL(triggered()), this, SLOT(handleDeleteRow()));
    addRowAction = new QAction("Append row", contextMenu);
    connect(addRowAction, SIGNAL(triggered()), this, SLOT(handleAddRow()));
    contextMenu->addAction(setNullAction);
    contextMenu->addAction(deleteRowAction);
    contextMenu->addAction(addRowAction);

    loadingOverlay = new LoadingOverlay{this};
    loadingOverlay->hide();
}

void TableView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) {
    QTreeView::closeEditor(editor, hint);

    QModelIndex idx = currentIndex();
    QModelIndex nextIndex;

    if(hint == QAbstractItemDelegate::EditNextItem)
        nextIndex = model()->index(idx.row(), idx.column() + 1, idx.parent());

    if(hint == QAbstractItemDelegate::EditPreviousItem)
        nextIndex = model()->index(idx.row(), idx.column() - 1, idx.parent());

    if(nextIndex.isValid()) {
        if((model()->flags(nextIndex) & Qt::ItemIsEditable)) {
            setCurrentIndex(nextIndex);
            edit(nextIndex);
        } else {
            QTreeView::closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
        }
    }
}

void TableView::adjustColumnSizes(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
    for(int i = topLeft.column(); i < bottomRight.column(); ++i)
        resizeColumnToContents(i);
}

void TableView::setModel(QAbstractItemModel *m) {
    if(model() == m)
        return;

    if(model()) {
        disconnect(model());
        for(QHash<int, QWidget*>& h : foreignTableViews)
            qDeleteAll(h);
        foreignTableViews.clear();
    }

    if(m) {
        connect(m, &QAbstractItemModel::rowsAboutToBeRemoved, [=](){showLoadingOverlay(true);});
        connect(m, &QAbstractItemModel::rowsRemoved, [=](){showLoadingOverlay(false);});
        connect(m, &QAbstractItemModel::modelAboutToBeReset, [=](){showLoadingOverlay(true);});
        connect(m, &QAbstractItemModel::modelReset, [=](){showLoadingOverlay(false);});
        connect(m, &QAbstractItemModel::dataChanged, this, &TableView::adjustColumnSizes);
    }

    QTreeView::setModel(m);
    if(m) // this maybe works better after the data has been loaded?
        adjustColumnSizes(m->index(0,0),m->index(m->rowCount()-1,m->columnCount()-1));
}

QWidget* TableView::createChildTable(const QModelIndex& index) {
    QFrame * frame = new QFrame(this);
    frame->setLayout(new QVBoxLayout());
    frame->layout()->setSpacing(0);

    QLabel* label = new QLabel(frame);
    QFont fnt("monospace");
    fnt.setBold(true);
    label->setFont(fnt);
    ForeignKey fk = index.data(ForeignKeyRole).value<ForeignKey>();
    label->setText("SELECT * FROM `" + fk.table + "` WHERE `" + fk.column + "` = '" + index.data().toString() + "'");
    frame->layout()->addWidget(label);

    TableView* view = new TableView(frame);
    view->setMaximumHeight(QWIDGETSIZE_MAX);
    view->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    TableModel* m = static_cast<TableModel*>(model());
    if(m) {
        TableModel* childModel = new TableModel(*m, fk.table);
        view->setModel(childModel);
        childModel->describe(Filter{fk.column, "=", index.data().toString()});
    }
    frame->layout()->addWidget(view);

    return frame;
}

void TableView::resizeEvent(QResizeEvent *event) {
    QTreeView::resizeEvent(event);
    loadingOverlay->setGeometry(viewport()->geometry());
}

void TableView::showLoadingOverlay(bool show) {
    loadingOverlay->setVisible(show);
}

void TableView::openMenu(QPoint p) {
    QModelIndex index = indexAt(p);
    contextMenu->popup(viewport()->mapToGlobal(p));
    setNullAction->setEnabled(index.isValid());
    deleteRowAction->setEnabled(index.isValid());
}

void TableView::handleSetNull() {
    model()->setData(currentIndex(), QVariant());
}

void TableView::handleDeleteRow() {
    QSet<int> rows;
    for(const QModelIndex& i : selectedIndexes())
        rows << i.row();
    ((SqlModel*) model())->deleteRows(rows);
    clearSelection();
}

void TableView::handleAddRow() {
    int newRow = model()->rowCount();
    model()->insertRow(newRow);
    QModelIndex newRowIndex = model()->index(newRow, 0);
    setCurrentIndex(newRowIndex);
    edit(newRowIndex);
}

void TableView::toggleForeignTable(const QModelIndex& index) {
    QModelIndex first = index.sibling(index.row(), 0);
    if(isExpanded(first)) {
        collapse(first);
        model()->setData(first, -1, ExpandedColumnIndexRole);

    } else {
        model()->setData(first, index.column(), ExpandedColumnIndexRole);
        QModelIndex child = model()->index(0, 0, first);
        setFirstColumnSpanned(child.row(), first, true);
        QWidget* subTable = createChildTable(index);

        if(indexWidget(child) != subTable)
            setIndexWidget(child, subTable);
        expand(first);
    }

    // force the recursive resizing of any parent TableViews in response to this change
    for(QWidget* p = this; p; p = p->parentWidget()) {
        TableView* tv = qobject_cast<TableView*>(p);
        if(tv) {
            tv->updateGeometry();
            tv->scheduleDelayedItemsLayout();
        }
    }

}
