/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "mainpanel.h"

#include "connectionwidget.h"
#include "sshdbconnection.h"
#include "viewtoolbar.h"
#include "querypanel.h"
#include "filteredpagedtableview.h"
#include "tablelist.h"
#include "querylog.h"

#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QModelIndex>
#include <QSqlTableModel>
#include <QListWidgetItem>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLineEdit>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>
#include <QDebug>
#include <QThread>

MainPanel::MainPanel(QWidget* parent) :
    QWidget(parent),
    db_(nullptr)
{
    backgroundWorker_ = new QThread;
    backgroundWorker_->start();

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    { // content, structure, info, query toolbar
        toolbar_ = new ViewToolBar(this);
        connect(toolbar_, SIGNAL(panelChanged(ViewToolBar::Panel)), this, SLOT(openPanel(ViewToolBar::Panel)));
        connect(toolbar_, SIGNAL(disconnect()), this, SLOT(disconnectDb()));
        layout->addWidget(toolbar_);
    }

    { // the main (bottom) panel is either content/schema/query or setup
        { // widget for connection choosing/setup
            settings_ = new ConnectionWidget(this);
            layout->addWidget(settings_);
            connect(settings_, SIGNAL(doConnect(QString)), this, SLOT(openConnection(QString)));
        }

        { // content widget is encapsulated in a splitter for the query log
            logSplit_ = new QSplitter(this);
            logSplit_->setOrientation(Qt::Vertical);
            { // content/schema/query widgets (top half of splitter)
                QWidget* actionPanel = new QWidget(this);
                QLayout* actionLayout = new QVBoxLayout(actionPanel);
                { // content and schema share a table selector in a splitter....
                    contentSchemaSplit_ = new QSplitter(actionPanel);
                    contentSchemaSplit_->setOrientation(Qt::Horizontal);
                    //splitView_->setHandleWidth(2);

                    tableChooser_ = new TableList(this);
                    connect(tableChooser_, SIGNAL(tableSelected(QString)), this, SLOT(tableChanged(QString)));
                    connect(tableChooser_, SIGNAL(addButtonClicked()), this, SLOT(addTable()));
                    connect(tableChooser_, SIGNAL(delButtonClicked()), this, SLOT(deleteTable()));
                    connect(tableChooser_, SIGNAL(refreshButtonClicked()), this, SLOT(refreshTables()));
                    contentSchemaSplit_->addWidget(tableChooser_);

                    { // the contents and structure table views (sharing the table list)
                        QWidget* w = new QWidget(this);
                        QBoxLayout* vlayout = new QVBoxLayout(w);
                        vlayout->setContentsMargins(0,0,0,0);
                        //vlayout->setSpacing(0);

                        content_ = new FilteredPagedTableView(w);
                        connect(content_, SIGNAL(foreignQuery(QString,QString,QVariant)), this, SLOT(jumpToQuery(QString,QString,QVariant)));
                        vlayout->addWidget(content_);

                        structure_ = new TableView(w);
                        structure_->hide(); // show only the contents by default
                        vlayout->addWidget(structure_);
                        w->setLayout(vlayout);
                        contentSchemaSplit_->addWidget(w);
                    }
                    // end of splitter widget
                    contentSchemaSplit_->setStretchFactor(1,4);
                    actionLayout->addWidget(contentSchemaSplit_);

                }
                { // ...but this doesn't appear in query mode
                    queryWidget_ = new QueryPanel(this);
                    queryWidget_->hide(); // it is of course still hidden by default
                    actionLayout->addWidget(queryWidget_);
                }
                logSplit_->addWidget(actionPanel);
            }
            { // query log (bottom half of splitter)
                queryLog_ = new QueryLog(this);
                logSplit_->addWidget(queryLog_);
            }
            layout->addWidget(logSplit_);
            logSplit_->setStretchFactor(0,5);
            logSplit_->setStretchFactor(1,1);
        }
    }

    emit nameChanged(this, "New Connection");
    toggleEditSettings(true);
}

MainPanel::~MainPanel() {
    backgroundWorker_->exit();
    backgroundWorker_->wait();
    delete backgroundWorker_;
}

void MainPanel::openPanel(ViewToolBar::Panel p) {
    if(p == ViewToolBar::PANEL_QUERY) {
        contentSchemaSplit_->hide();
        queryWidget_->show();
    } else {
        queryWidget_->hide();
        contentSchemaSplit_->show();
        content_->hide();
        structure_->hide();
        switch(p) {
        case ViewToolBar::PANEL_CONTENT:
            updateContentModel(tableChooser_->selectedTable());
            content_->show();
            break;
        case ViewToolBar::PANEL_STRUCTURE:
            updateSchemaModel(tableChooser_->selectedTable());
            structure_->show();
            break;
        default:
            break;
        }
    }
}
#include "sqlcontentmodel.h"
void MainPanel::updateContentModel(QString tableName) {
    bool isModelNew = false;
    SqlContentModel* model;
    QString key = db_->databaseName() + tableName;
    if(!db_->contentModels().contains(key)) {
        model = new SqlContentModel(*db_, tableName);
        db_->contentModels()[key] = model;
        isModelNew = true;
    } else
        model = qobject_cast<SqlContentModel*>(db_->contentModels()[key]);
    content_->setModel(model);
    if(isModelNew) {
        if(!jumpToTableFilter_.value.isNull())
            model->describe(jumpToTableFilter_);
        else
            model->describe();
    } else if(!jumpToTableFilter_.value.isNull())
        model->setFilter(jumpToTableFilter_);
    jumpToTableFilter_ = Filter{};
}
#include "sqlschemamodel.h"
void MainPanel::updateSchemaModel(QString tableName) {
    bool isModelNew = false;
    SqlSchemaModel* model;
    QString key = db_->databaseName() + tableName;
    if(!db_->schemaModels().contains(key)) {
        model = new SqlSchemaModel(*db_, tableName);
        db_->schemaModels()[key] = model;
        isModelNew = true;
    }
    model = qobject_cast<SqlSchemaModel*>(db_->schemaModels()[key]);
    structure_->setModel(model);
    if(isModelNew)
        model->describe();
}
void MainPanel::openConnection(QString name) {
    db_ = DbConnection::fromName(name);
    db_->moveToThread(backgroundWorker_);
    connect(db_, SIGNAL(queryExecuted(QString,QString)), queryLog_, SLOT(logQuery(QString,QString)));
qDebug() << "out: " << QThread::currentThreadId();
    connect(db_, SIGNAL(connectionSuccess()), this, SLOT(databaseConnected()));

    // todo put in databesConnected
    QSettings s;
    s.beginGroup(name);
    QString label = s.value("Name").toString();
    s.endGroup();

    emit nameChanged(this, label);






    connect(db_, SIGNAL(databaseChanged(QString)), this, SLOT(tableListChanged()));

    // todo show loading progress
    //db_->connect();
    QMetaObject::invokeMethod(db_, "connect", Qt::QueuedConnection);

}

void MainPanel::databaseConnected() {
    qDebug() << "in: " << QThread::currentThreadId();

    toolbar_->enableAll();
    queryWidget_->setDb(db_);

    toggleEditSettings(false);

    toolbar_->populateDatabases(db_->getDatabaseNames());
    if(!db_->getDatabaseName().isEmpty()) {
        toolbar_->setCurrentDatabase(db_->getDatabaseName());
        tableChooser_->setTableNames(db_->tables());
    }

    connect(toolbar_, SIGNAL(dbChanged(QString)), this, SLOT(dbChanged(QString)));

}

void MainPanel::tableListChanged() {
    tableChooser_->setTableNames(db_->tables());

}

void MainPanel::dbChanged(QString name) {
    content_->setModel(nullptr);
    structure_->setModel(nullptr);
    //db_->useDatabase(name);
    QMetaObject::invokeMethod(db_, "useDatabase", Q_ARG(QString, name));
    //tableChooser_->setTableNames(db_->tables());
}


void MainPanel::tableChanged(QString name) {
    if(structure_->isVisible())
        updateSchemaModel(name);
    if(content_->isVisible())
        updateContentModel(name);
    // todo this shouldn't be set for every change of table, maybe it can be done just once? Or functionality bought into class
    //connect(content_->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(changeSort(int,Qt::SortOrder)));
}

void MainPanel::changeSort(int col, Qt::SortOrder sort) {
    QSqlTableModel* m = static_cast<QSqlTableModel*>(content_->model());
    m->setSort(col, sort);
    m->select();
}

void MainPanel::toggleEditSettings(bool showSettings) {
    queryWidget_->setVisible(false);
    logSplit_->setVisible(!showSettings);
    settings_->setVisible(showSettings);
    toolbar_->enableAll(!showSettings);
}

void MainPanel::disconnectDb() {
    content_->setModel(0);
    structure_->setModel(0);
    queryLog_->clear();
    queryLog_->setRowCount(0);
    tableChooser_->setTableNames(QStringList());
    toggleEditSettings(true);
    if(db_) {
        disconnect(db_);
        QMetaObject::invokeMethod(db_, "cleanup", Qt::BlockingQueuedConnection);
        //backgroundWorker_->exit();
        delete db_;
        db_ = 0;
    }
}

void MainPanel::addTable() {
    QString name = QInputDialog::getText(this, "Create Table", "Enter a name for the new table");
    if(!name.isEmpty()) {
        db_->createTable(name);
        refreshTables();
    }
}

void MainPanel::deleteTable() {
    QString current = tableChooser_->selectedTable();
    if(!current.isNull() && QMessageBox::warning(this, QString("Delete Table"), "Are you sure? This action cannot be undone", QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        db_->deleteTable(current);
        content_->setModel(nullptr);
        structure_->setModel(nullptr);
        refreshTables();
    }
}

void MainPanel::refreshTables() {
    tableChooser_->setTableNames(db_->tables());
}

void MainPanel::jumpToQuery(QString table, QString column, QVariant value) {
qDebug() << "jump to " << table << " with " << column << " = " << value;
    jumpToTableFilter_.column = column;
    jumpToTableFilter_.operation = "=";
    jumpToTableFilter_.value = value.toString();
    //content_->setFilter(column, "=", value);
    tableChooser_->setCurrentTable(table);
}
