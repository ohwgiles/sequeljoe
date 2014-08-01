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
#include <QPushButton>
#include <QLabel>

TableList::TableList(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    { // widget for filtering tables
        filterInput_ = new QLineEdit(this);
        QHBoxLayout* filterLayout = new QHBoxLayout();
        filterLayout->setContentsMargins(0,0,0,0);
        filterLayout->addWidget(new QLabel("Filter:", this));
        filterLayout->addWidget(filterInput_);
        layout->addLayout(filterLayout);
    }

    { // main table list widget
        tableItems_ = new QStringListModel(this);

        QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
        proxy->setSourceModel(tableItems_);
        tables_ = new QListView(this);
        tables_->setEditTriggers(QListView::NoEditTriggers);
        tables_->setModel(proxy);

        connect(filterInput_, SIGNAL(textChanged(QString)), this, SLOT(filterTextChanged(QString)));
        connect(tables_->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChanged(QModelIndex)));

        layout->addWidget(tables_);
    }

    { // widget with "Add", "Remove" and "Refresh" actions
        QBoxLayout* bar = new QHBoxLayout();
        bar->setContentsMargins(0,0,0,0);

        QPushButton* addTable = new QPushButton("New Table", this);
        connect(addTable, SIGNAL(clicked()), this, SIGNAL(addButtonClicked()));
        bar->addWidget(addTable);

        QPushButton* delFavourite = new QPushButton("Delete Table", this);
        connect(delFavourite, SIGNAL(clicked()), this, SIGNAL(delButtonClicked()));
        bar->addWidget(delFavourite);

        QPushButton* refresh = new QPushButton("Refresh", this);
        connect(refresh, SIGNAL(clicked()), this, SIGNAL(refreshButtonClicked()));
        bar->addWidget(refresh);

        layout->addLayout(bar);
    }
}

void TableList::selectionChanged(QModelIndex index) {
    if(index.isValid())
        emit tableSelected(index.data().toString());
}

QString TableList::selectedTable() const {
    return tables_->currentIndex().data().toString();
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
