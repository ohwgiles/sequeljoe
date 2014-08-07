/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "filteredpagedtableview.h"
#include "sqlcontentmodel.h"

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

    table_ = new TableView(this);
    layout->addWidget(table_);

    // proxy the signal
    connect(table_, SIGNAL(foreignQuery(QString,QString,QVariant)), this, SIGNAL(foreignQuery(QString,QString,QVariant)));

    { // widget containing a toolbar with filter and pagination options
        QHBoxLayout* bar = new QHBoxLayout();
        bar->setContentsMargins(0,0,0,0);

        prev_ = new QPushButton("<", this);
        next_ = new QPushButton(">", this);
        pageNum_ = new QLabel(this);

        QWidget* spacer = new QWidget(this);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        filterColumns_ = new QComboBox(this);
        filterOperation_ = new QComboBox(this);
        filterText_ = new QLineEdit(this);

        filterRun_ = new QPushButton("Apply", this);
        connect(filterRun_, SIGNAL(clicked()), this, SLOT(runFilter()));

        filterClear_ = new QPushButton("Reset", this);
        connect(filterClear_, SIGNAL(clicked()), this, SLOT(clearFilter()));

        filterOperation_->addItems(filterOperations());
        bar->addWidget(filterColumns_);
        bar->addWidget(filterOperation_);
        bar->addWidget(filterText_);
        bar->addWidget(filterRun_);
        bar->addWidget(filterClear_);
        bar->addWidget(spacer);
        bar->addWidget(prev_);
        bar->addWidget(pageNum_);
        bar->addWidget(next_);

        layout->addLayout(bar);
    }
}

QStringList FilteredPagedTableView::filterOperations() const {
    QStringList ops;
    ops << "=" << "!=" << ">" << ">=" << "<" << "<=" << "CONTAINS" << "IN" << "LIKE";
    return ops;
}

void FilteredPagedTableView::setModel(QAbstractItemModel *m) {
    if(m == table_->model())
        return;

    disconnect(this, SLOT(updatePagination(int,int,int)));
    disconnect(prev_, SIGNAL(clicked()));
    disconnect(next_, SIGNAL(clicked()));
    filterColumns_->clear();
    filterText_->clear();
    table_->setModel(m);

    if(m) {
        connect(m, SIGNAL(pagesChanged(int,int,int)), this, SLOT(updatePagination(int,int,int)));
        connect(prev_, SIGNAL(clicked()), m, SLOT(prevPage()));
        connect(next_, SIGNAL(clicked()), m, SLOT(nextPage()));

        for(int i = 0; i < m->columnCount(); ++i)
            filterColumns_->addItem(m->headerData(i, Qt::Horizontal).toString());
        filterColumns_->setCurrentText(m->data(QModelIndex(), FilterColumnRole).toString());
        filterOperation_->setCurrentText(m->data(QModelIndex(), FilterOperationRole).toString());
        filterText_->setText(m->data(QModelIndex(), FilterValueRole).toString());
        //refreshModel();
    }
}

void FilteredPagedTableView::updatePagination(int firstRow, int rowsInPage, int totalRecords) {
    prev_->setDisabled(firstRow == 0);
    next_->setDisabled(firstRow + rowsInPage >= totalRecords);
    int last = (firstRow + rowsInPage < totalRecords ? firstRow+rowsInPage : totalRecords);
    if(totalRecords == 0)
        pageNum_->setText("No Records");
    else
        pageNum_->setText("Rows " + QString::number(firstRow+1) + " to " + QString::number(last) + " of " + QString::number(totalRecords));
}

void FilteredPagedTableView::clearFilter() {
    filterText_->setText(QString());
    runFilter();
}
#include <QDebug>
void FilteredPagedTableView::setFilter(QString column, QString operation, QVariant value) {
    filterColumns_->setCurrentText(column);
    filterOperation_->setCurrentText(operation);
    filterText_->setText(value.toString());
    runFilter();
}

void FilteredPagedTableView::runFilter() {
    qDebug() << __PRETTY_FUNCTION__;
    model()->setData(QModelIndex(), filterColumns_->currentText(), FilterColumnRole);
    model()->setData(QModelIndex(), filterOperation_->currentText(), FilterOperationRole);
    model()->setData(QModelIndex(), filterText_->text(), FilterValueRole);
    refreshModel();
}

void FilteredPagedTableView::refreshModel() {
    table_->showLoadingOverlay(true);
    QEvent event{QEvent::Type(RefreshEvent)};
    model()->event(&event);
}
