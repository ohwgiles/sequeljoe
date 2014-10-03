/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "mainwindow.h"

#include "tabwidget.h"
#include "mainpanel.h"

#include <QSqlTableModel>
#include <QSettings>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent) :
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
        tabs = new TabWidget(centralWidget);
        connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged(int)));
        tabs->setCurrentIndex(0);
        layout->addWidget(tabs);
        connect(tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(handleTabClosed(int)));
        connect(tabs, SIGNAL(newTab()), this, SLOT(newTab()));
    }

    setCentralWidget(centralWidget);
    newTab();

    QSettings s;
    restoreGeometry(s.value("geometry").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent*) {
    QSettings s;
    s.setValue("geometry", saveGeometry());
}

void MainWindow::newTab() {
    MainPanel* w = new MainPanel(this);
    connect(w, SIGNAL(nameChanged(QWidget*,QString)), this, SLOT(updateTabName(QWidget*,QString)));
    tabs->insertTab(tabs->lastActiveIndex()+1, w, QString("New Connection"));
    tabs->setCurrentWidget(w);
}

void MainWindow::handleTabChanged(int index) {
    setWindowTitle(tabs->tabText(index) + " - SequelJoe");
}

void MainWindow::handleTabClosed(int index) {
    MainPanel* panel = (MainPanel*) tabs->widget(index);
    panel->disconnectDb();
    if(tabs->count() > 1) {
        // if this is the last real tab (not including the + tab), we have to select
        // the previous tab first, so that the + tab is not auto-selected, creating
        // a new tab
        if(index == 0)
            newTab();
        if(index > 0 && index + 2 == tabs->count()) {
            tabs->setCurrentIndex(index-1);
        }
        delete panel;
    }
}

void MainWindow::updateTabName(QWidget* tab, QString name) {
    tabs->setTabText(tabs->indexOf(tab), name);
    handleTabChanged(tabs->indexOf(tab));
}
