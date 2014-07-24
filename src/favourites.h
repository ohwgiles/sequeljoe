/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_FAVOURITES_H_
#define _SEQUELJOE_FAVOURITES_H_

#include <QWidget>

class QListWidgetItem;
class QListWidget;
class QToolBar;

class Favourites : public QWidget
{
    Q_OBJECT
public:
    explicit Favourites(QWidget *parent = 0);
signals:
    void favouriteSelected(QString name);
public slots:
    void updateName(QString name);

private slots:
    void indexChanged(QListWidgetItem*);
    void addButtonClicked();
    void deleteButtonClicked();
private:
    QListWidget* list_;
    QLayout* bar_;
};

#endif // _SEQUELJOE_FAVOURITES_H_
