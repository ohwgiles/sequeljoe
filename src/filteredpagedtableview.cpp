#include "filteredpagedtableview.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QToolBar>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QLabel>
#include "sqlcontentmodel.h"

FilteredPagedTableView::FilteredPagedTableView(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
table_ = new TableView(this);
    layout->addWidget(table_);

    QHBoxLayout* bar = new QHBoxLayout(this);
    //bar->addWidget(new QPushButton("hi"));
    bar->setSpacing(4);

    prev_ = new QPushButton("<", this);
    next_ = new QPushButton(">", this);
    pageNum_ = new QLabel(this);
    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    bar->addWidget(spacer);
    bar->addWidget(prev_);
    bar->addWidget(pageNum_);
    bar->addWidget(next_);

    layout->addLayout(bar);

    setLayout(layout);
}

void FilteredPagedTableView::setModel(QAbstractItemModel *m) {
    disconnect(this, SLOT(updatePagination(int,int,int)));
    connect(m, SIGNAL(pagesChanged(int,int,int)), this, SLOT(updatePagination(int,int,int)));
    disconnect(prev_, SIGNAL(clicked()));
    connect(prev_, SIGNAL(clicked()), m, SLOT(prevPage()));
    disconnect(next_, SIGNAL(clicked()));
    connect(next_, SIGNAL(clicked()), m, SLOT(nextPage()));
    SqlContentModel* sm = (SqlContentModel*) m;
    table_->setModel(m);
    sm->select();
}

void FilteredPagedTableView::updatePagination(int firstRow, int rowsInPage, int totalRecords)
{
    prev_->setDisabled(firstRow == 0);
    next_->setDisabled(firstRow + rowsInPage >= totalRecords);
    int last = (firstRow + rowsInPage < totalRecords ? firstRow+rowsInPage : totalRecords) - 1;
    pageNum_->setText("Rows " + QString::number(firstRow) + " to " + QString::number(last) + " of " + QString::number(totalRecords));
    //repaint();
    //table_->model()->
}


#include <QPaintEvent>
#include <QPainter>
