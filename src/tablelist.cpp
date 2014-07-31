/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tablelist.h"

#include <QListView>
#include <QLineEdit>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QLabel>

TableList::TableList(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* vlayout = new QVBoxLayout(this);

    vlayout->setContentsMargins(0,0,0,0);
    filterInput_ = new QLineEdit(this);
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->setContentsMargins(0,0,0,0);
    filterLayout->addWidget(new QLabel("Filter:", this));
    filterLayout->addWidget(filterInput_);
    vlayout->addLayout(filterLayout);

    tableItems_ = new QStringListModel(this);

    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(tableItems_);
    tables_ = new QListView(this);
    tables_->setEditTriggers(QListView::NoEditTriggers);
    tables_->setModel(proxy);

    connect(filterInput_, SIGNAL(textChanged(QString)), this, SLOT(filterTextChanged(QString)));
    connect(tables_->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChanged(QModelIndex)));

    vlayout->addWidget(tables_);

    setLayout(vlayout);
}

void TableList::selectionChanged(QModelIndex index) {
    if(index.isValid())
        emit tableSelected(index.data().toString());
}

void TableList::filterTextChanged(QString text) {
    QSortFilterProxyModel* model = static_cast<QSortFilterProxyModel*>(tables_->model());
    tables_->setCurrentIndex(QModelIndex());
    tables_->clearSelection();
    model->setFilterFixedString(text);
}

void TableList::setTableNames(QStringList names) {
    tableItems_->setStringList(names);
}
