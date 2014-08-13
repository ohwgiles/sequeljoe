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
    group_ = new QActionGroup(this);
    setIconSize(QSize(20,20));
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // adding a dummy space widget appears to even out the toolbar
    addWidget(new QWidget(this));

    dbSelect_ = new QComboBox(this);
    dbSelect_->setMinimumWidth(150);
    connect(dbSelect_, SIGNAL(activated(int)), this, SLOT(dbComboModified(int)));
    addWidget(dbSelect_);

    addAction(":content", "Content", SLOT(showContent()))->setChecked(true);
    addAction(":schema", "Schema", SLOT(showStructure()));
    addAction(":query", "Query", SLOT(showQuery()));

    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    addWidget(spacer);

    addAction(":disconnect", "Disconnect", SIGNAL(disconnect()))->setCheckable(false);
}

QAction* ViewToolBar::addAction(QString icon, QString label, const char* slot) {
    QAction* a = QToolBar::addAction(QIcon(icon), label, this, slot);
    a->setEnabled(false);
    a->setCheckable(true);
    // allows checkable exclusivity
    group_->addAction(a);
    return a;
}

void ViewToolBar::enableAll(bool enabled) {
    actions().first()->setChecked(true);
    for(QAction* a : actions())
        a->setEnabled(enabled);
    populateDatabases(QStringList());
}

void ViewToolBar::populateDatabases(QStringList names) {
    QString current = dbSelect_->currentText();
    dbSelect_->clear();
    dbSelect_->addItem("Choose Database...");
    dbSelect_->insertSeparator(1);
    dbSelect_->addItems(names);
    if(!current.isEmpty())
        dbSelect_->setCurrentText(current);
}

void ViewToolBar::setCurrentDatabase(QString name) {
    dbSelect_->setCurrentText(name);
}

void ViewToolBar::dbComboModified(int indx) {
    if(indx > 0)
        emit dbChanged(dbSelect_->currentText());
}
