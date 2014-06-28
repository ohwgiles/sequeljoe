/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "mainwindow.h"

#include <libssh2.h>
#include "sshdbconnection.h"
#include <QtSql/QSqlTableModel>
#include "mainpanel.h"
#include <QDebug>
#include <QSettings>
#include <QMenuBar>
#include <QVBoxLayout>

#include "tabwidget.h"


#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    this->setWindowIcon(QIcon(":icon"));

    { // menu bar
        QMenuBar* menuBar = new QMenuBar(this);
        menuBar->setNativeMenuBar(true);
        menuBar->addAction("test", this, "");
        setMenuBar(menuBar);
    }

    QWidget* w = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(w);
    //layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    {   // tab bar
        tabs_ = new TabWidget(w);
        //QPushButton* newTab = new QPushButton("+", this);
        //QStyle* s = newTab->style();
//newTab->setPalette();
        //newTab->setStyle();
        //tabs_->setCornerWidget(newTab);
//tabs_->setTabBar(new TabBarPlus(tabs_));
        //tabWidget->setTabShape(QTabWidget::Triangular);
        tabs_->setDocumentMode(true);
        tabs_->setMovable(true);
        tabs_->setTabsClosable(true);
        connect(tabs_, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged(int)));
        tabs_->setCurrentIndex(0);
        layout->addWidget(tabs_);
        connect(tabs_, SIGNAL(tabCloseRequested(int)), this, SLOT(handleTabClosed(int)));
        connect(tabs_, SIGNAL(newTab()), this, SLOT(newTab()));
        //newTab->setStyle(tabs_->style());
    }

    setCentralWidget(w);

    newTab();

    QSettings s;
    this->restoreGeometry(s.value("geometry").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *) {
    QSettings s;
    s.setValue("geometry", saveGeometry());
}

void MainWindow::newTab()
{
    MainPanel* w = new MainPanel(nullptr, this);
    connect(w, SIGNAL(nameChanged(QWidget*,QString)), this, SLOT(updateTabName(QWidget*,QString)));
    tabs_->insertTab(tabs_->lastActiveIndex()+1, w, QString("New Connection"));
    tabs_->setCurrentWidget(w);
}

void MainWindow::handleTabChanged(int index)
{
    //qDebug() << tabs_->indexOf(tabs_->previousInFocusChain());

    setWindowTitle("SequelJoe - " + tabs_->tabText(index));
}

void MainWindow::handleTabClosed(int index)
{
    MainPanel* panel = (MainPanel*) tabs_->widget(index);
    panel->disconnectDb();
    if(tabs_->count() > 1)
        delete panel;
        //tabs_->removeTab(index);
}

void MainWindow::updateTabName(QWidget * tab, QString name)
{
    qDebug() << "setting name to " << name;
    tabs_->setTabText(tabs_->indexOf(tab), name);
    setWindowTitle("SequelJoe - " + name);
}

MainWindow::~MainWindow()
{
    QSqlDatabase::removeDatabase("dbname");
}
