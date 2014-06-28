/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tabwidget.h"
#include <QTabBar>

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    lastIndex_ = -1;
    setDocumentMode(true);
    setMovable(true);
    setTabsClosable(true);

    empty_ = new QWidget(this);
    QTabWidget::addTab(empty_,"+");

    QWidget* empty = new QWidget(this);
    empty->setMaximumSize(QSize(0,0));
#if __APPLE__ // should really be by style
    tabBar()->setTabButton(0, QTabBar::LeftSide, empty);
#else
    tabBar()->setTabButton(0, QTabBar::RightSide, empty);
#endif
    connect(tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(plusToEnd()));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(checkNewTab(int)));
}

void TabWidget::plusToEnd() {
    tabBar()->moveTab(indexOf(empty_), count()-1);
}

void TabWidget::checkNewTab(int idx) {
    if(widget(idx) == empty_)
        emit newTab();
    else
        lastIndex_ = idx;
}
