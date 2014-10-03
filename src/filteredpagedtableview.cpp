/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "filteredpagedtableview.h"

#include "tableview.h"
#include "roles.h"
#include "sqlmodel.h" // for RefreshEvent

#include <QVBoxLayout>
#include <QPushButton>
#include <QToolBar>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QToolButton>

FilteredPagedTableView::FilteredPagedTableView(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    table = new TableView(this);
    layout->addWidget(table);

    { // widget containing a toolbar with filter and pagination options
        QHBoxLayout* bar = new QHBoxLayout();
        bar->setContentsMargins(0,0,0,0);

        prev = new QPushButton("<", this);
        next = new QPushButton(">", this);
        pageNum = new QLabel(this);

        QWidget* spacer = new QWidget(this);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        filterColumns = new QComboBox(this);
        filterOperation = new QComboBox(this);
        filterText = new QLineEdit(this);

        filterRun = new QPushButton("Apply", this);
        connect(filterRun, SIGNAL(clicked()), this, SLOT(runFilter()));

        filterClear = new QPushButton("Reset", this);
        connect(filterClear, SIGNAL(clicked()), this, SLOT(clearFilter()));

        filterOperation->addItems(filterOperations());
        bar->addWidget(filterColumns);
        bar->addWidget(filterOperation);
        bar->addWidget(filterText);
        bar->addWidget(filterRun);
        bar->addWidget(filterClear);
        bar->addWidget(spacer);
        bar->addWidget(prev);
        bar->addWidget(pageNum);
        bar->addWidget(next);

        layout->addLayout(bar);
    }
}

QStringList FilteredPagedTableView::filterOperations() const {
    QStringList ops;
    ops << "=" << "!=" << ">" << ">=" << "<" << "<=" << "CONTAINS" << "IN" << "LIKE";
    return ops;
}

void FilteredPagedTableView::setModel(QAbstractItemModel *m) {
    if(m == table->model())
        return;

    disconnect(this, SLOT(updatePagination(int,int,int)));
    disconnect(this, SLOT(populateFilter()));
    disconnect(prev, SIGNAL(clicked()));
    disconnect(next, SIGNAL(clicked()));

    filterColumns->clear();
    filterText->clear();
    table->setModel(m);

    if(m) {
        connect(m, SIGNAL(pagesChanged(int,int,int)), this, SLOT(updatePagination(int,int,int)));
        connect(prev, SIGNAL(clicked()), m, SLOT(prevPage()));
        connect(next, SIGNAL(clicked()), m, SLOT(nextPage()));
        connect(m, SIGNAL(selectFinished()), this, SLOT(populateFilter()));
    }
}

QAbstractItemModel* FilteredPagedTableView::model() const {
    return table->model();
}

void FilteredPagedTableView::updatePagination(int firstRow, int rowsInPage, int totalRecords) {
    prev->setDisabled(firstRow == 0);
    next->setDisabled(firstRow + rowsInPage >= totalRecords);
    int last = (firstRow + rowsInPage < totalRecords ? firstRow+rowsInPage : totalRecords);
    if(totalRecords == 0)
        pageNum->setText("No Records");
    else
        pageNum->setText("Rows " + QString::number(firstRow+1) + " to " + QString::number(last) + " of " + QString::number(totalRecords));
}

void FilteredPagedTableView::populateFilter() {
    for(int i = 0; i < model()->columnCount(); ++i)
        filterColumns->addItem(model()->headerData(i, Qt::Horizontal).toString());
    filterColumns->setCurrentText(model()->data(QModelIndex(), FilterColumnRole).toString());
    filterOperation->setCurrentText(model()->data(QModelIndex(), FilterOperationRole).toString());
    filterText->setText(model()->data(QModelIndex(), FilterValueRole).toString());
}

void FilteredPagedTableView::clearFilter() {
    filterText->setText(QString());
    runFilter();
}

void FilteredPagedTableView::setFilter(QString column, QString operation, QVariant value) {
    filterColumns->setCurrentText(column);
    filterOperation->setCurrentText(operation);
    filterText->setText(value.toString());
    runFilter();
}

void FilteredPagedTableView::runFilter() {
    model()->setData(QModelIndex(), filterColumns->currentText(), FilterColumnRole);
    model()->setData(QModelIndex(), filterOperation->currentText(), FilterOperationRole);
    model()->setData(QModelIndex(), filterText->text(), FilterValueRole);
    refreshModel();
}

void FilteredPagedTableView::refreshModel() {
    table->showLoadingOverlay(true);
    QEvent event{QEvent::Type(RefreshEvent)};
    model()->event(&event);
}
