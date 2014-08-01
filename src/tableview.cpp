/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tableview.h"

#include "tablecell.h"
#include "sqlcontentmodel.h"

#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QLayout>

TableView::TableView(QWidget *parent) :
    QTableView(parent)
{
    setContentsMargins(0,0,0,0);

    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setShowGrid(false);

    TableCell* tc = new TableCell;
    connect(tc, SIGNAL(goToForeignEntry(QModelIndex)), this, SLOT(handleRequestForeignKey(QModelIndex)));
    setItemDelegate(tc);

    horizontalHeader()->setSortIndicatorShown(true);
    horizontalHeader()->setFixedHeight(verticalHeader()->minimumSectionSize());
    verticalHeader()->setDefaultSectionSize(verticalHeader()->minimumSectionSize());
    verticalHeader()->setVisible(false);

    // context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(openMenu(QPoint)));
    ctxMenu_ = new QMenu(this);
    nullAction_ = new QAction("Set to NULL", ctxMenu_);
    connect(nullAction_, SIGNAL(triggered()), this, SLOT(handleSetNull()));
    deleteRowAction_ = new QAction("Delete row", ctxMenu_);
    connect(deleteRowAction_, SIGNAL(triggered()), this, SLOT(handleDeleteRow()));
    addRowAction_ = new QAction("Append row", ctxMenu_);
    connect(addRowAction_, SIGNAL(triggered()), this, SLOT(handleAddRow()));
    ctxMenu_->addAction(nullAction_);
    ctxMenu_->addAction(deleteRowAction_);
    ctxMenu_->addAction(addRowAction_);
}

void TableView::openMenu(QPoint p) {
    QModelIndex index = indexAt(p);
    ctxMenu_->popup(viewport()->mapToGlobal(p));
    nullAction_->setEnabled(index.isValid());
    deleteRowAction_->setEnabled(index.isValid());
}

void TableView::handleSetNull() {
    model()->setData(currentIndex(), QVariant());
}

void TableView::handleDeleteRow() {
    QSet<int> rows;
    for(const QModelIndex& i : selectedIndexes())
        rows << i.row();
    for(int i : rows)
        model()->removeRow(i);
    QEvent event{QEvent::Type(RefreshEvent)};
    model()->event(&event);
}

void TableView::handleAddRow() {
    int newRow = model()->rowCount();
    model()->insertRow(newRow);
    edit(model()->index(newRow, 0));
}

void TableView::handleRequestForeignKey(const QModelIndex& index) {
    QString table = index.data(ForeignKeyTableRole).toString();
    QString column = index.data(ForeignKeyColumnRole).toString();
    QVariant value = index.data();
    emit foreignQuery(table, column, value);
}
