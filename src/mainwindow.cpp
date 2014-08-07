/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "mainwindow.h"


#include "sshdbconnection.h"
#include "tabwidget.h"
#include "mainpanel.h"

#include <libssh2.h>
#include <QSqlTableModel>
#include <QSettings>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowIcon(QIcon(":icon"));

    { // menu bar
        QMenuBar* menuBar = new QMenuBar(this);
        //QMenu* db = menuBar->addMenu("Database");
        //db->addAction("test", this, );
        setMenuBar(menuBar);
    }

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    { // tab bar
        tabs_ = new TabWidget(centralWidget);
        connect(tabs_, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged(int)));
        tabs_->setCurrentIndex(0);
        layout->addWidget(tabs_);
        connect(tabs_, SIGNAL(tabCloseRequested(int)), this, SLOT(handleTabClosed(int)));
        connect(tabs_, SIGNAL(newTab()), this, SLOT(newTab()));
    }

    setCentralWidget(centralWidget);
    newTab();

    QSettings s;
    this->restoreGeometry(s.value("geometry").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *) {
    QSettings s;
    s.setValue("geometry", saveGeometry());
}

void MainWindow::newTab() {
    MainPanel* w = new MainPanel(this);
    connect(w, SIGNAL(nameChanged(QWidget*,QString)), this, SLOT(updateTabName(QWidget*,QString)));
    tabs_->insertTab(tabs_->lastActiveIndex()+1, w, QString("New Connection"));
    tabs_->setCurrentWidget(w);
}

void MainWindow::handleTabChanged(int index) {
    setWindowTitle(tabs_->tabText(index) + " - SequelJoe");
}

void MainWindow::handleTabClosed(int index) {
    MainPanel* panel = (MainPanel*) tabs_->widget(index);
    panel->disconnectDb();
    if(tabs_->count() > 1) {
        // if this is the last real tab (not including the + tab), we have to select
        // the previous tab first, so that the + tab is not auto-selected, creating
        // a new tab
        if(index == 0)
            newTab();
        if(index > 0 && index + 2 == tabs_->count()) {
            tabs_->setCurrentIndex(index-1);
        }
        delete panel;
    }
}

void MainWindow::updateTabName(QWidget * tab, QString name) {
    tabs_->setTabText(tabs_->indexOf(tab), name);
    handleTabChanged(tabs_->indexOf(tab));
}
