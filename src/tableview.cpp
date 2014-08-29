/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tableview.h"

#include "tablecell.h"
#include "sqlcontentmodel.h"
#include "loadingoverlay.h"
//#include "sqlmodel.h"

#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QLayout>
#include <QResizeEvent>
#include <QLineEdit>
#include <QLabel>
TableView::TableView(QWidget *parent) :
    QTreeView(parent)
{
    setContentsMargins(0,0,0,0);

    setAlternatingRowColors(true);
//    setSelectionBehavior(QAbstractItemView::SelectRows);
//    setShowGrid(false);

    TableCell* tc = new TableCell(this);
    connect(tc, SIGNAL(goToForeignEntry(QModelIndex)), this, SLOT(handleRequestForeignKey(QModelIndex)));
    setItemDelegate(tc);
setRootIsDecorated(false);

//    horizontalHeader()->setSortIndicatorShown(true);
//    horizontalHeader()->setFixedHeight(verticalHeader()->minimumSectionSize());
//    horizontalHeader()->setHighlightSections(false);
//    verticalHeader()->setDefaultSectionSize(verticalHeader()->minimumSectionSize());
//    verticalHeader()->setVisible(false);
connect(this, SIGNAL(collapsed(QModelIndex)), tc, SLOT(handleCollapse(QModelIndex)));
connect(this, SIGNAL(expanded(QModelIndex)), tc, SLOT(handleExpand(QModelIndex)));
    // context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(openMenu(QPoint)));
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

    loadingOverlay_ = new LoadingOverlay{this};
    loadingOverlay_->hide();
}
#include <QDebug>
QSize TableView::sizeHint() const {
    QSize sz = QTreeView::sizeHint();
//    QTreeView* area = qobject_cast<QTreeView*>(parent());

//    if(area) {
//        qDebug() << area->maximumHeight();
//        sz.setHeight(area->maximumHeight());
//    }
    int height = header()->height();
    if(model()) {
    qDebug() << "rowheights:"<<rowHeight(model()->index(0,0));
    qDebug() << rowHeight(model()->index(1,0));
}

//    QModelIndex idx = rootIndex();
//    while(idx.isValid()) {
//        height += rowHeight(idx);
//        idx = idx.child(0,0);
//    }
    sz.setHeight(height);
    return sz;
}

void TableView::setModel(QAbstractItemModel *m) {
    if(model() == m)
        return;

    if(model()) {
        disconnect(model());
        for(QHash<int, QWidget*>& h : foreignTableWidgets_)
            qDeleteAll(h);
        foreignTableWidgets_.clear();
    }

    if(m) {
    connect(m, &QAbstractItemModel::modelAboutToBeReset, [=](){showLoadingOverlay(true);});
    connect(m, &QAbstractItemModel::modelReset, [=](){
        showLoadingOverlay(false);
    });
    connect(m, &QAbstractItemModel::dataChanged, [=](const QModelIndex& topLeft, const QModelIndex& bottomRight){
        for(int i = topLeft.column(); i < bottomRight.column(); ++i)
            resizeColumnToContents(i);
    });
}
    QTreeView::setModel(m);

    if(SqlContentModel* contentModel = dynamic_cast<SqlContentModel*>(m)) {
        contentModel->subwidgetFactory_ = this;
    }
}

QWidget* TableView::createTableView(const QModelIndex& index) {
    if(!foreignTableWidgets_.contains(index.row()) || !foreignTableWidgets_.value(index.row()).contains(index.column())) {

        QFrame * f = new QFrame(this);
        TableView* view = new TableView(this);
        view->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        //view->setMaximumHeight(99999);
        qDebug() << "created view";
        SqlContentModel* m = static_cast<SqlContentModel*>(model());
        //setFirstColumnSpanned(index.row(), index, true);
        SqlContentModel* childModel = new SqlContentModel(m->db(), index.data(ForeignKeyTableRole).toString());
        view->setModel(childModel);
        qDebug() << index.data(ForeignKeyColumnRole).toString() << "=" << index.data().toString();
        childModel->describe(Filter{index.data(ForeignKeyColumnRole).toString(), "=", index.data().toString()});
        qDebug() << "returning view";
        f->setLayout(new QVBoxLayout());
        //f->layout()->setContentsMargins(0,0,0,0);
        f->layout()->setSpacing(0);
        f->layout()->addWidget(new QLabel(index.data(ForeignKeyTableRole).toString() + ": " + index.data(ForeignKeyColumnRole).toString()));
        f->layout()->addWidget(view);


        foreignTableWidgets_[index.row()][index.column()] = f;
//            QHash<int,QWidget*> h = foreignTableWidgets_[index.row()];
//            h.insert(index.column(), subwidgetFactory_->createTableView(index));
//            foreignTableWidgets_.insert(index.row(), h);
        //foreignTableWidgets_[index.row()].insert(index.column(), subwidgetFactory_->createTableView(index));
    }

    return foreignTableWidgets_.value(index.row()).value(index.column());

}

void TableView::resizeEvent(QResizeEvent *event) {
    QTreeView::resizeEvent(event);
    loadingOverlay_->setGeometry(geometry());
}

void TableView::showLoadingOverlay(bool show) {
    loadingOverlay_->setVisible(show);
}

void TableView::openMenu(QPoint p) {
    QModelIndex index = indexAt(p);
    ctxMenu_->popup(viewport()->mapToGlobal(p));
    nullAction_->setEnabled(index.isValid());
    deleteRowAction_->setEnabled(index.isValid());
}

void TableView::handleSetNull() {
    model()->setData(currentIndex(), QVariant());
}

void TableView::handleDeleteRow() {
    QSet<int> rows;
    for(const QModelIndex& i : selectedIndexes())
        rows << i.row();
    ((AbstractSqlModel*) model())->deleteRows(rows);
    clearSelection();
//    QEvent event{QEvent::Type(RefreshEvent)};
//    model()->event(&event);
}

void TableView::handleAddRow() {
    int newRow = model()->rowCount();
    model()->insertRow(newRow);
    edit(model()->index(newRow, 0));
}

void TableView::handleRequestForeignKey(const QModelIndex& index) {
//    QString table = index.data(ForeignKeyTableRole).toString();
//    QString column = index.data(ForeignKeyColumnRole).toString();
//    QVariant value = index.data();
    qDebug() << index;
    QModelIndex first = index.sibling(index.row(), 0);
    qDebug() << first;
    qDebug() << "exanded:"<<isExpanded(first);
    if(isExpanded(first)) {
        collapse(first);
        model()->setData(first, -1, ExpandedColumnIndexRole);
    }else {
        model()->setData(first, index.column(), ExpandedColumnIndexRole);
    expand(first);
    }
    //emit foreignQuery(table, column, value);
}
