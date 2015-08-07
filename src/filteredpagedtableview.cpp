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
#include "tablecell.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QToolBar>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QToolButton>
#include <QAbstractProxyModel>
#include <QIdentityProxyModel>
#include <QMenu>
#include <QInputDialog>

class PivotProxy : public QAbstractProxyModel {
public:
    PivotProxy(QObject *parent = 0) : QAbstractProxyModel(parent)
    {
    }
    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override {
        return sourceModel() ? index(sourceIndex.column(), sourceIndex.row()+1) : QModelIndex();
    }
    QModelIndex mapToSource(const QModelIndex& proxyIndex) const override {
        return (sourceModel() && proxyIndex.column()>0) ? sourceModel()->index(proxyIndex.column()-1, proxyIndex.row()) : QModelIndex();
    }
    QModelIndex index(int row, int col, const QModelIndex& parent = QModelIndex()) const override {
        return createIndex(row, col);
    }
    QModelIndex parent(const QModelIndex& sourceIndex) const override {
        return QModelIndex();
    }
    int rowCount(const QModelIndex&) const override {
        return sourceModel() ? sourceModel()->columnCount() : 0;
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const {
        return QVariant();
    }
    bool hasChildren(const QModelIndex &parent) const override {
        if(!parent.isValid()) return true;
        return false;
    }
    int columnCount(const QModelIndex&) const override {
        return sourceModel() ? sourceModel()->rowCount() + 1 : 0;
    }
    QVariant data(const QModelIndex& idx, int role) const override {
        if(sourceModel() && idx.column() == 0) {
            return sourceModel()->headerData(idx.row(), Qt::Horizontal, role);
        } else
            return QAbstractProxyModel::data(idx, role);
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        if(index.column() == 0) return false;
        if(sourceModel()) return sourceModel()->setData(mapToSource(index),value,role);
        return false;
    }
};

FilteredPagedTableView::FilteredPagedTableView(QWidget *parent) :
    QWidget(parent),
    isPivot(false),
    pivotModel(new PivotProxy(this)),
    rowsPerPage(100)
{
    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    table = new TableView(this);
    layout->addWidget(table);

    { // widget containing a toolbar with filter and pagination options
        QHBoxLayout* bar = new QHBoxLayout();
        bar->setContentsMargins(0,0,0,0);


        first = new QPushButton("<<", this);
        first->setMaximumWidth(first->sizeHint().height());
        prev = new QPushButton("<", this);
        prev->setMaximumWidth(prev->sizeHint().height());
        next = new QPushButton(">", this);
        next->setMaximumWidth(next->sizeHint().height());
        last = new QPushButton(">>", this);
        last->setMaximumWidth(last->sizeHint().height());
        pageNum = new QLabel(this);

        QMenu* viewMenu = new QMenu(this);
        viewMenu->addAction("Pivot", this, SLOT(setPivotView(bool)))->setCheckable(true);
        viewMenu->addAction("Set rows per page", this, SLOT(setRowsPerPage()));
        view = new QPushButton("View");
        qobject_cast<QPushButton*>(view)->setMenu(viewMenu);


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
        bar->addWidget(new QLabel("Where: ", this));
        bar->addWidget(filterColumns);
        bar->addWidget(filterOperation);
        bar->addWidget(filterText);
        bar->addWidget(filterRun);
        bar->addWidget(filterClear);
        bar->addWidget(spacer);
        bar->addWidget(first);
        bar->addWidget(prev);
        bar->addWidget(pageNum);
        bar->addWidget(next);
        bar->addWidget(last);
        bar->addWidget(view);

        layout->addLayout(bar);
    }

    setDisabled(true);
}

QStringList FilteredPagedTableView::filterOperations() const {
    QStringList ops;
    ops << "=" << "!=" << ">" << ">=" << "<" << "<=" << "CONTAINS" << "IN" << "LIKE";
    return ops;
}

void FilteredPagedTableView::setPivotView(bool v) {
    if(v == isPivot)
        return;

    if(v) {
        pivotModel->setSourceModel(model());
        qobject_cast<TableCell*>(table->itemDelegate())->setFirstColumnIsHeader(true);
        table->setModel(pivotModel);
        table->setWordWrap(true);
    } else {
        table->setHeaderHidden(false);
        qobject_cast<TableCell*>(table->itemDelegate())->setFirstColumnIsHeader(false);
        table->setModel(pivotModel->sourceModel());
    }

    isPivot = v;
}

void FilteredPagedTableView::setRowsPerPage() {
    bool ok;
    int rows = QInputDialog::getInt(this, "Table View", "Rows per page", rowsPerPage, 1, 1000, 1, &ok);
    if(ok) {
        if(SqlModel* m = qobject_cast<SqlModel*>(model())) {
            m->setRowsPerPage(rows);
            rowsPerPage = rows;
        }
    }
}

void FilteredPagedTableView::setModel(QAbstractItemModel *m) {
    if(m == table->model())
        return;

    disconnect(this, SLOT(updatePagination(int,int,int)));
    disconnect(this, SLOT(populateFilter()));
    disconnect(first, SIGNAL(clicked()), 0, 0);
    disconnect(prev, SIGNAL(clicked()), 0, 0);
    disconnect(next, SIGNAL(clicked()), 0, 0);
    disconnect(last, SIGNAL(clicked()), 0, 0);

    filterColumns->clear();
    filterText->clear();

    if(isPivot) {
        pivotModel->setSourceModel(m);
        // SOMETHING IS WRONG HERE:
        // This nasty construct below is used get all the signal/slot goodness
        // in TableView::setModel, but then using the pivotModel as the actual
        // data. Moreover it bypasses the early return in that function that
        // would prevent simply using pivotModel->setSourceModel.
        // PROBLEM is, if you switch to a table for which there is no pre-existing
        // model WHILE isPivot is true, TableView::sizeHintForColumn always returns
        // -1, and the columns are not resized correctly. Even if there is an
        // existing model, it seems that incorrect values are returned. The
        // columns look OK but are not correct (until you click Reset)
        table->setModel(m);
        table->QTreeView::setModel(pivotModel);
    } else {
        table->setModel(m);
    }

    if(m) {
        connect(m, SIGNAL(pagesChanged(int,int,int)), this, SLOT(updatePagination(int,int,int)));
        connect(first, SIGNAL(clicked()), m, SLOT(firstPage()));
        connect(prev, SIGNAL(clicked()), m, SLOT(prevPage()));
        connect(next, SIGNAL(clicked()), m, SLOT(nextPage()));
        connect(last, SIGNAL(clicked()), m, SLOT(lastPage()));
        connect(m, SIGNAL(selectFinished()), this, SLOT(populateFilter()));
        if(SqlModel* sm = qobject_cast<SqlModel*>(model())) {
            sm->setRowsPerPage(rowsPerPage,false);
            sm->signalPagination();
        }
        populateFilter();
    }

    setEnabled(m);
}

QAbstractItemModel* FilteredPagedTableView::model() const {
    return isPivot ? pivotModel->sourceModel() : table->model();
}

void FilteredPagedTableView::updatePagination(int firstRow, int rowsInPage, int totalRecords) {
    first->setDisabled(firstRow == 0);
    prev->setDisabled(firstRow == 0);
    next->setDisabled(rowsInPage == 0 || (totalRecords != -1 && firstRow + rowsInPage >= totalRecords));
    last->setDisabled(rowsInPage == 0 || totalRecords == -1 || firstRow + rowsInPage >= totalRecords);
    int last = firstRow + rowsInPage;
    if(totalRecords == 0 || (firstRow == 0 && rowsInPage == 0))
        pageNum->setText("No Records");
    else if(totalRecords == -1)
        pageNum->setText("Rows " + QString::number(firstRow+1) + " to " + QString::number(last));
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
