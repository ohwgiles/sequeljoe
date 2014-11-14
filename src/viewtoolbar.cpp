/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */

#include "viewtoolbar.h"

#include <QAction>
#include <QSpacerItem>
#include <QComboBox>
#include <QLabel>

ViewToolBar::ViewToolBar(QWidget *parent) :
    QToolBar(parent)
{
    group = new QActionGroup(this);
    setIconSize(QSize(20,20));
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // adding a dummy space widget appears to even out the toolbar
    addWidget(new QWidget(this));

    dbSelect = new QComboBox(this);
    dbSelect->setMinimumWidth(150);
    connect(dbSelect, SIGNAL(activated(int)), this, SLOT(dbComboModified(int)));
    addWidget(dbSelect);

    viewActions.append(addExclusiveAction(":content", "Content", SLOT(showContent())));
    viewActions.append(addExclusiveAction(":schema", "Schema", SLOT(showStructure())));
    viewActions.append(addExclusiveAction(":query", "Query", SLOT(showQuery())));

    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    addWidget(spacer);

    back = QToolBar::addAction("<", this, SIGNAL(historyBack()));
    back->setEnabled(false);
    forward = QToolBar::addAction(">", this, SIGNAL(historyForward()));
    forward->setEnabled(false);

    viewActions.append(addAction(QIcon(":disconnect"), "Disconnect", this, SIGNAL(disconnect())));
}

void ViewToolBar::setHistoryButtonsEnabled(bool aback, bool aforward) {
    back->setEnabled(aback);
    forward->setEnabled(aforward);
}

void ViewToolBar::triggerPanelOpen(Panel p) {
    int i = 2 + int(p - PANEL_CONTENT);
    actions().at(i)->trigger();
}

QAction* ViewToolBar::addExclusiveAction(QString icon, QString label, const char* slot) {
    QAction* a = QToolBar::addAction(QIcon(icon), label, this, slot);
    a->setEnabled(false);
    a->setCheckable(true);
    // allows checkable exclusivity
    group->addAction(a);
    return a;
}

void ViewToolBar::enableViewActions(bool enabled) {
    actions().first()->setChecked(true);
    for(QAction* a : viewActions)
        a->setEnabled(enabled);
    populateDatabases(QStringList());
}

void ViewToolBar::populateDatabases(QStringList names) {
    QString current = dbSelect->currentText();
    dbSelect->clear();
    dbSelect->addItem("Choose Database...");
    dbSelect->insertSeparator(1);
    dbSelect->addItems(names);
    if(!current.isEmpty())
        dbSelect->setCurrentText(current);
}

void ViewToolBar::setCurrentDatabase(QString name) {
    dbSelect->setCurrentText(name);
}

void ViewToolBar::dbComboModified(int indx) {
    if(indx > 0)
        emit dbChanged(dbSelect->currentText());
}
