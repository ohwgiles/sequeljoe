/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
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

#include "mainpanel.h"
#include "connectionwidget.h"
#include "sshdbconnection.h"
#include "viewtoolbar.h"
#include "querypanel.h"
#include "tableview.h"
#include "tablelist.h"

MainPanel::MainPanel(DbConnection *db, QWidget* parent) :
    QWidget(parent),
    db_(db)
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

    { // this separates the table list from the table content/structure
        splitView_ = new QSplitter(this);
        splitView_->setOrientation(Qt::Horizontal);
        splitView_->setHandleWidth(2);

        tableChooser_ = new TableList(this);
        connect(tableChooser_, SIGNAL(tableSelected(QString)), this, SLOT(tableChanged(QString)));
        splitView_->addWidget(tableChooser_);

        { // the contents and structure table views (sharing the table list)
            QWidget* w = new QWidget(this);
            QBoxLayout* vlayout = new QVBoxLayout(w);
            vlayout->setContentsMargins(0,0,0,0);
            vlayout->setSpacing(0);

            content_ = new TableView(w);
            vlayout->addWidget(content_);

            structure_ = new TableView(w);
            structure_->hide(); // show only the contents by default
            vlayout->addWidget(structure_);
            w->setLayout(vlayout);
            splitView_->addWidget(w);
        }
        // end of splitter widget
        splitView_->setStretchFactor(1,4);
        layout->addWidget(splitView_);
    }

    { // there is no sense in showing the table list for the query view, so raise it a level
        queryWidget_ = new QueryPanel(this);
        queryWidget_->hide(); // it is of course still hidden by default
        layout->addWidget(queryWidget_);
    }

    {
        settings_ = new ConnectionWidget(this);
        layout->addWidget(settings_);
        connect(settings_, SIGNAL(doConnect(QString)), this, SLOT(openConnection(QString)));
    }

    this->setLayout(layout);

    emit nameChanged(this, "New Connection");
    toggleEditSettings(db_ == 0);
}

void MainPanel::openPanel(ViewToolBar::Panel p)
{
    if(p == ViewToolBar::PANEL_QUERY) {
        splitView_->hide();
        queryWidget_->show();
    } else {
        queryWidget_->hide();
        splitView_->show();
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


#include <QDebug>
void MainPanel::openConnection(QString name) {
    // todo close any existing connection, or prevent connection reuse
    db_ = DbConnection::fromName(name);
    if(!db_) {
        // failed to create db specification: todo notify the user
        return;
    }

    connect(db_, SIGNAL(connectionSuccess()), this, SLOT(firstConnectionMade()));

    // todo show loading progress
    db_->connect();
    toggleEditSettings(false);
    qDebug() << "setting name to " << name;
    QSettings s;
    s.beginGroup(name);
    QString label = s.value("Name").toString();
    s.endGroup();

    emit nameChanged(this, label);
}



void MainPanel::firstConnectionMade()
{
    // todo terminate any loading progress widget

    toolbar_->enableAll();
    queryWidget_->setDb(db_);

    toolbar_->populateDatabases(db_->getDatabaseNames());
    if(!db_->getDatabaseName().isEmpty())
        toolbar_->setCurrentDatabase(db_->getDatabaseName());

    connect(toolbar_, SIGNAL(dbChanged(QString)), this, SLOT(dbChanged(QString)));


    disconnect(db_, SIGNAL(connectionSuccess()), this, SLOT(firstConnectionMade()));
    connect(db_, SIGNAL(connectionSuccess()), this, SLOT(connectionMade()));
    // todo below should be done all times, above should only be done once


    connectionMade();
}

void MainPanel::connectionMade()
{//if(!db_->isOpen()) return;

    tableChooser_->setTableNames(db_->getTableNames());

}

void MainPanel::dbChanged(QString name)
{
    content_->setModel(0);
    structure_->setModel(0);
    db_->close();
    db_->setDbName(name);
    db_->connect();
}

void MainPanel::tableChanged(QString name)
{
    QSqlTableModel* m = db_->getTableModel(name);
    structure_->setModel(db_->getStructureModel(name));
    content_->setModel(m);
    // todo this shouldn't be set for every change of table, maybe it can be done just once? Or functionality bought into class
    connect(content_->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(changeSort(int,Qt::SortOrder)));
}

void MainPanel::changeSort(int col, Qt::SortOrder sort)
{
    QSqlTableModel* m = static_cast<QSqlTableModel*>(content_->model());
    m->setSort(col, sort);
    m->select();
}

void MainPanel::toggleEditSettings(bool showSettings) {
    queryWidget_->setVisible(false);
    splitView_->setVisible(!showSettings);
    settings_->setVisible(showSettings);
    toolbar_->enableAll(!showSettings);
}

void MainPanel::disconnectDb()
{
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

