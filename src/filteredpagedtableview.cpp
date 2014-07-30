#include "filteredpagedtableview.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QToolBar>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QLabel>
#include "sqlcontentmodel.h"
#include <QComboBox>
#include <QLineEdit>
#include <QToolButton>
FilteredPagedTableView::FilteredPagedTableView(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    //layout->setSpacing(0);
table_ = new TableView(this);
    layout->addWidget(table_);
//table_->setAttribute(Qt::WA_LayoutUsesWidgetRect);
//    QWidget* barw = new QWidget(this);
//    barw->setContentsMargins(0,0,0,0);

    QHBoxLayout* bar = new QHBoxLayout();
    //layout->setMargin(0);
    bar->setContentsMargins(0,0,0,0);
    //bar->setSpacing(0);
    //bar->addWidget(new QPushButton("hi"));
    //bar->setSpacing(9);//style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));

    prev_ = new QPushButton("<", this);
    next_ = new QPushButton(">", this);
    pageNum_ = new QLabel(this);
    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    filterColumns_ = new QComboBox(this);
    filterOperation_ = new QComboBox(this);
    filterText_ = new QLineEdit(this);
    filterRun_ = new QPushButton(this);
    //this->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    filterRun_->setText("Apply");
    connect(filterRun_, SIGNAL(clicked()), this, SLOT(runFilter()));
    filterClear_ = new QPushButton("Reset", this);
    connect(filterClear_, SIGNAL(clicked()), this, SLOT(clearFilter()));
    QStringList ops;
    ops << "=" << "!=" << "contains" << "LIKE";
    filterOperation_->addItems(ops);
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

    //setLayout(layout);
}

void FilteredPagedTableView::setModel(QAbstractItemModel *m) {
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
    for(int i = 0; i < m->columnCount(); ++i) {

        filterColumns_->addItem(m->headerData(i, Qt::Horizontal).toString());
    }
    filterColumns_->setCurrentText(m->data(QModelIndex(), FilterColumnRole).toString());
    filterOperation_->setCurrentText(m->data(QModelIndex(), FilterOperationRole).toString());
    filterText_->setText(m->data(QModelIndex(), FilterValueRole).toString());
    refreshModel();
    }

}

void FilteredPagedTableView::updatePagination(int firstRow, int rowsInPage, int totalRecords)
{
    prev_->setDisabled(firstRow == 0);
    next_->setDisabled(firstRow + rowsInPage >= totalRecords);
    int last = (firstRow + rowsInPage < totalRecords ? firstRow+rowsInPage : totalRecords);
    if(totalRecords == 0) {
        pageNum_->setText("No Records");
    } else {
        pageNum_->setText("Rows " + QString::number(firstRow+1) + " to " + QString::number(last) + " of " + QString::number(totalRecords));
    }
    //repaint();
    //table_->model()->
}

void FilteredPagedTableView::clearFilter()
{
    filterText_->setText(QString());
    runFilter();
}

void FilteredPagedTableView::runFilter()
{
    model()->setData(QModelIndex(), filterColumns_->currentText(), FilterColumnRole);
    model()->setData(QModelIndex(), filterOperation_->currentText(), FilterOperationRole);
    model()->setData(QModelIndex(), filterText_->text(), FilterValueRole);
    refreshModel();
}

void FilteredPagedTableView::refreshModel() {
    QEvent event{QEvent::Type(RefreshEvent)};
    model()->event(&event);
}

#include <QPaintEvent>
#include <QPainter>
