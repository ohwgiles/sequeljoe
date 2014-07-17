#include "tablelist.h"

#include <QListView>
#include <QLineEdit>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

TableList::TableList(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* vlayout = new QVBoxLayout(this);

    filterInput_ = new QLineEdit(this);
    vlayout->addWidget(filterInput_);

    tableItems_ = new QStringListModel(this);

    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(tableItems_);
    tables_ = new QListView(this);
    tables_->setEditTriggers(QListView::NoEditTriggers);
    tables_->setModel(proxy);

    connect(filterInput_, SIGNAL(textChanged(QString)), this, SLOT(filterTextChanged(QString)));
    connect(tables_->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChanged(QModelIndex)));

    vlayout->setContentsMargins(0,0,0,0);
    vlayout->setSpacing(0);
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
    //tables_->selectionModel()->select(tables_->currentIndex(), QItemSelectionModel::Deselect);
    model->setFilterFixedString(text);
}

void TableList::setTableNames(QStringList names) {
    tableItems_->setStringList(names);
}
