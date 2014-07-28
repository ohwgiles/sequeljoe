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

    QHBoxLayout* bar = new QHBoxLayout(this);
    //bar->addWidget(new QPushButton("hi"));
    bar->setSpacing(4);

    prev_ = new QPushButton("<", this);
    next_ = new QPushButton(">", this);

    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    bar->addWidget(spacer);
    bar->addWidget(prev_);
    bar->addWidget(next_);

    layout->addLayout(bar);

    setLayout(layout);
}

void FilteredPagedTableView::previousPage() {
}

#include <QPaintEvent>
#include <QPainter>
