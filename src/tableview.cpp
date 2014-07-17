/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tableview.h"
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include "tablecell.h"
TableView::TableView(QWidget *parent) :
    QTableView(parent)
{
    // todo remove this, but resize columns to content on load complete (allowing manual column resize)
    //horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setSortIndicatorShown(true);
this->setAlternatingRowColors(true);
    //tableView->resizeRowsToContents();
    //horizontalHeader()->setStretchLastSection(true);
    verticalHeader()->setDefaultSectionSize(verticalHeader()->minimumSectionSize());
    verticalHeader()->setVisible(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(openMenu(QPoint)));
setShowGrid(false);
setItemDelegate(new TableCell());
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

#include <QDebug>
void TableView::openMenu(QPoint p) {
    QModelIndex index = indexAt(p);
    ctxMenu_->popup(viewport()->mapToGlobal(p));
    nullAction_->setEnabled(index.isValid());
    deleteRowAction_->setEnabled(index.isValid());
//    if(index.isValid()) {
//        QMenu* menu = new QMenu(this);
//        menu->addAction("test");
//        menu->addAction("test2");
//        menu->popup(viewport()->mapToGlobal(p));
//        //menu->exec(p);
//    }
}

#include <QSqlTableModel>
void TableView::handleSetNull() {
    model()->setData(currentIndex(), QVariant());
}

void TableView::handleDeleteRow() {
    model()->removeRow(currentIndex().row());
}

void TableView::handleAddRow() {
    int newRow = model()->rowCount();
    model()->insertRow(newRow);
    edit(model()->index(newRow, 0));
}
