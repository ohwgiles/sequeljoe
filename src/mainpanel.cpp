/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "mainpanel.h"

#include "connectionwidget.h"
#include "viewtoolbar.h"
#include "querypanel.h"
#include "filteredpagedtableview.h"
#include "schemaview.h"
#include "tablelist.h"
#include "querylog.h"
#include "loadingoverlay.h"
#include "tablemodel.h"
#include "schemamodel.h"
#include "sqlhighlighter.h"

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
#include <QThread>
#include <QDialog>
#include <QPlainTextEdit>

MainPanel::MainPanel(QWidget* parent) :
    QWidget(parent),
    db(nullptr)
{
    backgroundWorker = new QThread;
    backgroundWorker->start();

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    { // content, structure, info, query toolbar
        toolbar = new ViewToolBar(this);
        connect(toolbar, SIGNAL(panelChanged(ViewToolBar::Panel)), this, SLOT(openPanel(ViewToolBar::Panel)));
        connect(toolbar, SIGNAL(disconnect()), this, SLOT(disconnectDb()));
        connect(toolbar, SIGNAL(historyBack()), this, SLOT(historyBack()));
        connect(toolbar, SIGNAL(historyForward()), this, SLOT(historyForward()));
        layout->addWidget(toolbar);
    }

    { // the main (bottom) panel is either content/schema/query or setup
        { // widget for connection choosing/setup
            settingsPanel = new ConnectionWidget(this);
            layout->addWidget(settingsPanel);
            connect(settingsPanel, SIGNAL(doConnect(QString)), this, SLOT(openConnection(QString)));
        }

        { // content widget is encapsulated in a splitter for the query log
            splitLogViewer = new QSplitter(this);
            splitLogViewer->setOrientation(Qt::Vertical);
            { // content/schema/query widgets (top half of splitter)
                QWidget* actionPanel = new QWidget(this);
                QLayout* actionLayout = new QVBoxLayout(actionPanel);
                { // content and schema share a table selector in a splitter....
                    splitTableChooser = new QSplitter(actionPanel);
                    splitTableChooser->setOrientation(Qt::Horizontal);
                    //splitView_->setHandleWidth(2);

                    tableChooser = new TableList(this);
                    connect(tableChooser, SIGNAL(tableSelected(QString)), this, SLOT(tableChanged(QString)));
                    connect(tableChooser, SIGNAL(addButtonClicked()), this, SLOT(addTable()));
                    connect(tableChooser, SIGNAL(delButtonClicked()), this, SLOT(deleteTable()));
                    connect(tableChooser, SIGNAL(refreshButtonClicked()), this, SLOT(refreshTables()));
                    connect(tableChooser, SIGNAL(showTableRequested()), this, SLOT(showCreateTable()));
                    splitTableChooser->addWidget(tableChooser);

                    { // the contents and structure table views (sharing the table list)
                        QWidget* w = new QWidget(this);
                        QBoxLayout* vlayout = new QVBoxLayout(w);
                        vlayout->setContentsMargins(0,0,0,0);
                        //vlayout->setSpacing(0);

                        contentView = new FilteredPagedTableView(w);
                        vlayout->addWidget(contentView);

                        schemaView = new SchemaView(w);
                        schemaView->hide(); // show only the contents by default
                        vlayout->addWidget(schemaView);
                        w->setLayout(vlayout);
                        splitTableChooser->addWidget(w);
                    }
                    // end of splitter widget
                    splitTableChooser->setStretchFactor(1,4);
                    actionLayout->addWidget(splitTableChooser);

                }
                { // ...but this doesn't appear in query mode
                    queryWidget = new QueryPanel(this);
                    queryWidget->hide(); // it is of course still hidden by default
                    actionLayout->addWidget(queryWidget);
                }
                splitLogViewer->addWidget(actionPanel);
            }
            { // query log (bottom half of splitter)
                queryLog = new QueryLog(this);
                splitLogViewer->addWidget(queryLog);
            }
            layout->addWidget(splitLogViewer);
            splitLogViewer->setStretchFactor(0,5);
            splitLogViewer->setStretchFactor(1,1);
        }
    }

    loadingOverlay = new LoadingOverlay{this};
    loadingOverlay->hide();


    //emit nameChanged(this, "New Connection");
    toggleEditSettings(true);
}

MainPanel::~MainPanel() {
    backgroundWorker->exit();
    backgroundWorker->wait();
    delete backgroundWorker;
}

void MainPanel::openPanel(ViewToolBar::Panel p, QString table) {
    if(p == ViewToolBar::PANEL_QUERY) {
        splitTableChooser->hide();
        queryWidget->show();
    } else {
        queryWidget->hide();
        splitTableChooser->show();
        contentView->hide();
        schemaView->hide();
        switch(p) {
        case ViewToolBar::PANEL_CONTENT:
            updateContentModel(table);
            contentView->show();
            break;
        case ViewToolBar::PANEL_STRUCTURE:
            updateSchemaModel(table);
            schemaView->show();
            break;
        default:
            break;
        }
    }
}

QString MainPanel::currentTable() const {
    return tableChooser->selectedTable();
}

void MainPanel::updateContentModel(QString tableName) {
    QString key = db->databaseName() + tableName;
    if(!contentModels.contains(key)) {
        TableModel* model = new TableModel(*db, tableName);
        contentModels[key] = model;
        contentView->setModel(model);
        model->describe();
    } else
        contentView->setModel(contentModels[key]);
}


void MainPanel::updateSchemaModel(QString tableName) {
    QString key = db->databaseName() + tableName;
    if(!schemaModels.contains(key)) {
        SqlModel* schema = new SqlSchemaModel(*db, tableName);
        connect(schema, SIGNAL(schemaModified(QString)), this, SLOT(deleteContentModel(QString)));
        SqlModel* index  = new SqlModel(*db);
        index->setQuery("SHOW INDEX FROM " + tableName);
        schemaModels[key] = {schema, index};
        schemaView->setModels(schema, index);
        schema->select();
        index->select();
    } else {
        SchemaModels& models = schemaModels[key];
        schemaView->setModels(models.schema, models.index);
    }
}

void MainPanel::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    loadingOverlay->setGeometry(geometry());
}

void MainPanel::deleteContentModel(QString table) {
    QString key = db->databaseName() + table;
    if(contentModels.contains(key)) {
        contentView->setModel(nullptr);
        delete contentModels[key];
        contentModels.remove(key);
    }
}

void MainPanel::openConnection(QString name) {
    loadingOverlay->show();
    QSettings s;
    s.beginGroup(name);

    db = new DbConnection(s);
    db->moveToThread(backgroundWorker);

    connect(db, SIGNAL(queryExecuted(QString,QString)), queryLog, SLOT(logQuery(QString,QString)));
    connect(db, SIGNAL(connectionSuccess()), this, SLOT(databaseConnected()));
    connect(db, SIGNAL(connectionFailed(QString)), this, SLOT(connectionFailed(QString)));
    connect(db, SIGNAL(confirmUnknownHost(QString,bool*)), this, SLOT(confirmUnknownHost(QString,bool*)), Qt::BlockingQueuedConnection);

    QString label = s.value("Name").toString();
    s.endGroup();

    emit nameChanged(this, label);

    connect(db, SIGNAL(databaseChanged(QString)), this, SLOT(tableListChanged()));

    QMetaObject::invokeMethod(db, "start", Qt::QueuedConnection);
}

void MainPanel::databaseConnected() {
    loadingOverlay->hide();
    toolbar->enableViewActions();

    toggleEditSettings(false);

    toolbar->populateDatabases(db->databaseNames());
    if(!db->databaseName().isEmpty()) {
        toolbar->setCurrentDatabase(db->databaseName());
    }
    tableChooser->setTableNames(db->tables());

    connect(toolbar, SIGNAL(dbChanged(QString)), this, SLOT(dbChanged(QString)));

    SqlModel* m = new SqlModel(*db);
    queryWidget->setModel(m);
}

void MainPanel::connectionFailed(QString reason) {
    if(!reason.isNull())
        QMessageBox::critical(this, "Connection failed", reason);
    loadingOverlay->hide();
}

void MainPanel::confirmUnknownHost(QString fingerprint, bool* ok) {
    if(QMessageBox::warning(this, "Unknown Server Host Key", "Server host key with fingerprint " + fingerprint + " does not exist in known_hosts file. Would you like to continue and add it?", QMessageBox::Yes, QMessageBox::Abort) == QMessageBox::Yes)
        *ok = true;
}

void MainPanel::tableListChanged() {
    tableChooser->setTableNames(db->tables());

}

void MainPanel::dbChanged(QString name) {
    if(db->databaseName() != name) {
        contentView->setModel(nullptr);
        schemaView->setModels(nullptr, nullptr);
        QMetaObject::invokeMethod(db, "useDatabase", Q_ARG(QString, name));
    }
}

void MainPanel::tableChanged(QString name) {
    ViewToolBar::Panel p;
    if(schemaView->isVisible()) {
        updateSchemaModel(name);
        p = ViewToolBar::PANEL_STRUCTURE;
    }
    if(contentView->isVisible()) {
        updateContentModel(name);
        p = ViewToolBar::PANEL_CONTENT;
    }
    if(tableChangedManually) {
        locationStack.resize(++locationStackPosition);
        locationStack.append({name,p});
        toolbar->setHistoryButtonsEnabled(locationStackPosition>0, false);
    }
}

void MainPanel::historyBack() {
    HistoryEntry he = locationStack[--locationStackPosition];
    toolbar->setHistoryButtonsEnabled(locationStackPosition>0, true);
    tableChangedManually = false;
    tableChooser->setCurrentTable(he.table);
    tableChangedManually = true;
}

void MainPanel::historyForward() {
    HistoryEntry he = locationStack[++locationStackPosition];
    toolbar->setHistoryButtonsEnabled(true, locationStackPosition<locationStack.count()-1);
    tableChangedManually = false;
    tableChooser->setCurrentTable(he.table);
    tableChangedManually = true;
}

void MainPanel::changeSort(int col, Qt::SortOrder sort) { // todo
    QSqlTableModel* m = static_cast<QSqlTableModel*>(contentView->model());
    m->setSort(col, sort);
    m->select();
}

void MainPanel::toggleEditSettings(bool showSettings) {
    queryWidget->setVisible(false);
    splitLogViewer->setVisible(!showSettings);
    settingsPanel->setVisible(showSettings);
    toolbar->enableViewActions(!showSettings);
}

void MainPanel::disconnectDb() {
    contentView->setModel(nullptr);
    schemaView->setModels(nullptr, nullptr);
    queryLog->clear();
    queryLog->setRowCount(0);
    tableChooser->setTableNames(QStringList());
    toggleEditSettings(true);
    if(db) {
        disconnect(db);
        for(SqlModel* m : contentModels)
            delete m;
        for(SchemaModels m : schemaModels) {
            delete m.schema;
            delete m.index;
        }
        contentModels.clear();
        schemaModels.clear();

        QMetaObject::invokeMethod(db, "cleanup", Qt::BlockingQueuedConnection);
        //backgroundWorker_->exit();
        delete db;
        db = 0;
    }
}

void MainPanel::addTable() {
    QString name = QInputDialog::getText(this, "Create Table", "Enter a name for the new table");
    if(!name.isEmpty()) {
        QMetaObject::invokeMethod(db, "createTable", Qt::BlockingQueuedConnection, Q_ARG(QString, name));
        refreshTables(); // todo fake it?
        tableChooser->setCurrentTable(name);
        toolbar->triggerPanelOpen(ViewToolBar::PANEL_STRUCTURE);
        //openPanel(ViewToolBar::PANEL_STRUCTURE, name);
    }
}

void MainPanel::deleteTable() {
    QString current = tableChooser->selectedTable();
    if(!current.isNull() && QMessageBox::warning(this, QString("Delete Table"), "Are you sure? This action cannot be undone", QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        QMetaObject::invokeMethod(db, "deleteTable", Qt::BlockingQueuedConnection, Q_ARG(QString, current));
        contentView->setModel(nullptr);
        schemaView->setModels(nullptr,nullptr);
        refreshTables();
    }
}

void MainPanel::showCreateTable() {
    QString current = tableChooser->selectedTable();
    if(!current.isNull()) {
        QString res;
        QMetaObject::invokeMethod(db, "queryCreateTable", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, res), Q_ARG(QString, current));
        QDialog* dialog = new QDialog(this);
        dialog->setWindowTitle("SHOW CREATE TABLE `" + current + "`");
        dialog->setLayout(new QVBoxLayout(dialog));
        QPlainTextEdit* txt = new QPlainTextEdit(dialog);
        dialog->layout()->addWidget(txt);
        new SqlHighlighter(txt->document());
        txt->document()->setPlainText(res);
        txt->setReadOnly(true);
        QRect g = window()->geometry();
        dialog->setGeometry(g.x()+g.width()/8, g.y()+g.height()/8,g.width()*3/4,g.height()*3/4);
        dialog->setModal(true);
        dialog->exec();
        delete dialog;
    }
}

void MainPanel::refreshTables() {
    QMetaObject::invokeMethod(db, "populateTables", Qt::BlockingQueuedConnection);
    tableChooser->setTableNames(db->tables());
}

