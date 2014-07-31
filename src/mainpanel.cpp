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
#include <QSettings>

MainPanel::MainPanel(QWidget* parent) :
    QWidget(parent)
{
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
                    contentSchemaSplit_->addWidget(tableChooser_);

                    { // the contents and structure table views (sharing the table list)
                        QWidget* w = new QWidget(this);
                        QBoxLayout* vlayout = new QVBoxLayout(w);
                        vlayout->setContentsMargins(0,0,0,0);
                        //vlayout->setSpacing(0);

                        content_ = new FilteredPagedTableView(w);
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
                QWidget* queryLog = new QWidget(this);
                logSplit_->addWidget(queryLog);
            }
            logSplit_->setStretchFactor(6,1);
            layout->addWidget(logSplit_);
        }
    }

    emit nameChanged(this, "New Connection");
    toggleEditSettings(true);
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
            content_->show();
            break;
        case ViewToolBar::PANEL_STRUCTURE:
            structure_->show();
            break;
        default:
            break;
        }
    }
}

void MainPanel::openConnection(QString name) {
    db_ = DbConnection::fromName(name);
    connect(db_, SIGNAL(connectionSuccess()), this, SLOT(firstConnectionMade()));

    // todo show loading progress
    toggleEditSettings(false);
    db_->connect();
    QSettings s;
    s.beginGroup(name);
    QString label = s.value("Name").toString();
    s.endGroup();

    emit nameChanged(this, label);
}

// this handler only runs on first connect, for example to get the list of databases
void MainPanel::firstConnectionMade() {
    // todo terminate any loading progress widget
    toolbar_->enableAll();
    queryWidget_->setDb(db_);

    toolbar_->populateDatabases(db_->getDatabaseNames());
    if(!db_->getDatabaseName().isEmpty())
        toolbar_->setCurrentDatabase(db_->getDatabaseName());

    connect(toolbar_, SIGNAL(dbChanged(QString)), this, SLOT(dbChanged(QString)));

    // replace this handler with one that runs only on database changes
    disconnect(db_, SIGNAL(connectionSuccess()), this, SLOT(firstConnectionMade()));
    connect(db_, SIGNAL(connectionSuccess()), this, SLOT(connectionMade()));

    connectionMade();
}

void MainPanel::connectionMade() {
    tableChooser_->setTableNames(db_->tables());
}

void MainPanel::dbChanged(QString name) {
    content_->setModel(0);
    structure_->setModel(0);
    db_->close();
    db_->setDbName(name);
    db_->connect();
}

void MainPanel::tableChanged(QString name) {
    structure_->setModel(db_->getStructureModel(name));
    content_->setModel(db_->getTableModel(name));
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
    disconnect(this, SLOT(firstConnectionMade()));
    content_->setModel(0);
    structure_->setModel(0);
    tableChooser_->setTableNames(QStringList());
    toggleEditSettings(true);
    if(db_) {
        db_->close();
        delete db_;
        db_ = 0;
    }
}

