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
    QModelIndex idx = currentIndex();
    QModelIndex nextIndex = idx;

    // submit if the user clicks away
    if(hint == QAbstractItemDelegate::NoHint) {
        return QTreeView::closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
    }

    // don't allow custom editors to be popped up during tabbing,
    // it's more trouble than it's worth
    // todo somehow make this work, omitting it is annoying on pgsql
    auto canEditThroughTabbing = [&](QModelIndex i) {
        return (model()->flags(i) & Qt::ItemIsEditable) && model()->data(i, EditorTypeRole) == SJCellEditDefault;
    };

    // find next editible cell if user tabs
    if(hint == QAbstractItemDelegate::EditNextItem) do {
        nextIndex = model()->index(nextIndex.row(), nextIndex.column() + 1, nextIndex.parent());
    } while(nextIndex.isValid() && !canEditThroughTabbing(nextIndex));

    if(hint == QAbstractItemDelegate::EditPreviousItem) do {
        nextIndex = model()->index(nextIndex.row(), nextIndex.column() - 1, nextIndex.parent());
    } while(nextIndex.isValid() && !canEditThroughTabbing(nextIndex));

    if(idx != nextIndex) { // EditNextItem || EditPreviousItem
        if(nextIndex.isValid()) {
            QTreeView::closeEditor(editor, QAbstractItemDelegate::NoHint);
            setCurrentIndex(nextIndex);
            edit(nextIndex);
        } else { // end of row, submit
            QTreeView::closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
        }
    } else {
        // parent behaviour for SubmitModelCache and RevertModelCache
        QTreeView::closeEditor(editor, hint);
    }
}

void TableView::adjustColumnSizes() {
    for(int i = 0; i < model()->columnCount(); ++i) {
        int sh = sizeHintForColumn(i);
        int cw = sh < 0 ? header()->sectionSizeHint(i) : qMax(sh, header()->sectionSizeHint(i));
        header()->resizeSection(i, qMin(cw, 250));
    }
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
    QTreeView::setModel(m);

    if(m) {
        connect(m, &QAbstractItemModel::rowsAboutToBeRemoved, [=](){showLoadingOverlay(true);});
        connect(m, &QAbstractItemModel::rowsRemoved, [=](){showLoadingOverlay(false);});
        connect(m, &QAbstractItemModel::modelAboutToBeReset, [=](){showLoadingOverlay(true);});
        connect(m, SIGNAL(modelReset()), this, SLOT(handleModelReset()), Qt::QueuedConnection);
    }

    if(m) // this maybe works better after the data has been loaded?
        adjustColumnSizes();
}

void TableView::handleModelReset() {
    showLoadingOverlay(false);
    adjustColumnSizes();
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
    label->setText("SELECT * FROM \"" + fk.refTable + "\" WHERE \"" + fk.refColumn + "\" = '" + index.data().toString() + "'");
    frame->layout()->addWidget(label);

    TableView* view = new TableView(frame);
    view->setMaximumHeight(QWIDGETSIZE_MAX);
    view->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    TableModel* m = static_cast<TableModel*>(model());
    if(m) {
        TableModel* childModel = new TableModel(*m, fk.refTable);
        view->setModel(childModel);
        childModel->describe(Filter{fk.refColumn, "=", index.data().toString()});
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
