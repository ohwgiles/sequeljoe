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
#include <QPushButton>
#include <QMessageBox>
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
        bar_ = new QHBoxLayout();//QToolBar(this);
        bar_->setContentsMargins(0,0,0,0);
        //bar_->setSpacing(20);
        QPushButton* addFavourite = new QPushButton("New Favourite", this);
        connect(addFavourite, SIGNAL(clicked()), this, SLOT(addButtonClicked()));
        bar_->addWidget(addFavourite);
        QPushButton* delFavourite = new QPushButton("Delete Favourite", this);
        connect(delFavourite, SIGNAL(clicked()), this, SLOT(deleteButtonClicked()));
        bar_->addWidget(delFavourite);
//        QWidget* spacer = new QWidget(this);
//        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        //bar_->addWidget(spacer);
        layout->addLayout(bar_);
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
    if(QMessageBox::warning(this, QString("Delete Favourite"), "Are you sure? This action cannot be undone", QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        QSettings s;
        QListWidgetItem* item = list_->currentItem();
        s.remove(item->data(Qt::UserRole).toString());
        delete item;
    }
}
