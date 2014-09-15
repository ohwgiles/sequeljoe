/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */

#include "favourites.h"

#include <QSettings>
#include <QListWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QDateTime>
#include <QPushButton>
#include <QMessageBox>
#include <QListWidgetItem>

Favourites::Favourites(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    { // widget containing list of saved connections
        list_ = new QListWidget(this);

        connect(list_, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(indexChanged(QListWidgetItem*)));
        layout->addWidget(list_);
    }

    { // widget with "Add" and "Remove" actions
        bar_ = new QHBoxLayout();
        bar_->setContentsMargins(0,0,0,0);

        QPushButton* addFavourite = new QPushButton("New Favourite", this);
        connect(addFavourite, SIGNAL(clicked()), this, SLOT(addButtonClicked()));
        bar_->addWidget(addFavourite);

        QPushButton* delFavourite = new QPushButton("Delete Favourite", this);
        connect(delFavourite, SIGNAL(clicked()), this, SLOT(deleteButtonClicked()));
        bar_->addWidget(delFavourite);

        layout->addLayout(bar_);
    }
}

void Favourites::populateFromConfig() {
    QSettings s;
    int count = 0;
    foreach(QString c, s.childGroups()) {
        if(!c.startsWith("Favourite_"))
            continue;
        s.beginGroup(c);
        QListWidgetItem* item = new QListWidgetItem(s.value("Name", "Unnamed").toString());
        item->setData(Qt::UserRole, c);
        list_->addItem(item);
        s.endGroup();
        count++;
    }

    // if there are no favourites, create an empty one
    if(count == 0)
        addButtonClicked();
    else // select the first one
        list_->setCurrentRow(0);
}

void Favourites::indexChanged(QListWidgetItem * item) {
    item->setSelected(true);
    emit favouriteSelected(item->data(Qt::UserRole).toString());
}

void Favourites::updateName(QString name) {
    list_->currentItem()->setText(name);
}

void Favourites::addButtonClicked() {
    QListWidgetItem* item = new QListWidgetItem("New Connection");
    item->setData(Qt::UserRole, "Favourite_" + QString::number(QDateTime::currentMSecsSinceEpoch()));
    list_->addItem(item);
    list_->setCurrentItem(item);
}

void Favourites::deleteButtonClicked() {
    if(QMessageBox::warning(this, QString("Delete Favourite"), "Are you sure? This action cannot be undone", QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        QSettings s;
        QListWidgetItem* item = list_->currentItem();
        s.remove(item->data(Qt::UserRole).toString());
        delete item;
    }
}
