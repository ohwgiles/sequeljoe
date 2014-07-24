#include "filteredpagedtableview.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QToolBar>
#include <QSqlTableModel>
#include <QSqlQuery>
FilteredPagedTableView::FilteredPagedTableView(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
table_ = new TableView(this);
    layout->addWidget(table_);

    QToolBar* bar = new QToolBar(this);
    //bar->addWidget(new QPushButton("hi"));

    bar->addAction("<", this, SLOT(previousPage()));
    layout->addWidget(bar);

    setLayout(layout);
}

void FilteredPagedTableView::previousPage() {
}

#include <QPaintEvent>
#include <QPainter>
