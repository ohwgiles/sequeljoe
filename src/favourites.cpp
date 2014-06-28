/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include <QSettings>
#include <QListWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QDateTime>

#include "favourites.h"
#include <QDebug>
Favourites::Favourites(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    { // setup list widget
        list_ = new QListWidget(this);
        QSettings s;
        int count = 0;
        foreach(QString c, s.childGroups()) {
            if(!c.startsWith("Favourite_")) continue;
            s.beginGroup(c);
            QListWidgetItem* item = new QListWidgetItem(s.value("Name", "Unnamed").toString()); //todo owner?
            item->setData(Qt::UserRole, c);
            list_->addItem(item);
            s.endGroup();
            count++;
        }

        connect(list_, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(indexChanged(QListWidgetItem*)));
        layout->addWidget(list_);

        if(count == 0)
            addButtonClicked();
//        list_->setCurrentRow(0);
//        indexChanged(list_->currentItem());
    }

    { // toolbar
        bar_ = new QToolBar(this);
        bar_->addAction("+", this, SLOT(addButtonClicked()));
        bar_->addAction("-", this, SLOT(deleteButtonClicked()));
        layout->addWidget(bar_);
    }
}


#include <QListWidgetItem>
void Favourites::indexChanged(QListWidgetItem * item)
{
    ///if(!item->isSelected()) return;
    qDebug() << item->data(Qt::DisplayRole);
    qDebug() << item->data(Qt::UserRole);
    item->setSelected(true);
    emit favouriteSelected(item->data(Qt::UserRole).toString());
}

void Favourites::updateName(QString name)
{
    list_->currentItem()->setText(name);
}

void Favourites::addButtonClicked()
{
    QListWidgetItem* item = new QListWidgetItem("New Connection"); //todo owner?
    item->setData(Qt::UserRole, "Favourite_" + QString::number(QDateTime::currentMSecsSinceEpoch()));
    list_->addItem(item);
}

void Favourites::deleteButtonClicked()
{
    QSettings s;
    QListWidgetItem* item = list_->currentItem();
    s.remove(item->data(Qt::UserRole).toString());
    delete item;
}
