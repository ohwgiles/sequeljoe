/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_MAINPANEL_H_
#define _SEQUELJOE_MAINPANEL_H_

#include <QtWidgets/QWidget>
#include <QModelIndex>

#include "viewtoolbar.h"

class ConnectionWidget;
class DbConnection;
class QTableView;
class QSqlTableModel;
class QListWidgetItem;
class QListView;
class QLineEdit;
class QSortFilterProxyModel;
class QueryPanel;
class QSplitter;
class TableList;

class FilteredPagedTableView;

class MainPanel : public QWidget
{
    Q_OBJECT
public:
    MainPanel(DbConnection* db = 0, QWidget *parent = 0);
    void loadSettings(QListWidgetItem* item);
signals:
    void nameChanged(QWidget*,QString);
public slots:
    void toggleEditSettings(bool);
    void connectionMade();
    void firstConnectionMade();

    void openConnection(QString name);
    void tableChanged(QString name);

    void changeSort(int, Qt::SortOrder);

    void openPanel(ViewToolBar::Panel);

    void disconnectDb();

    void dbChanged(QString);

private:
    DbConnection* db_;
    ConnectionWidget* settings_;
    QSplitter* splitView_;
    TableList* tableChooser_;
    FilteredPagedTableView* content_;
    QTableView* structure_;
    ViewToolBar* toolbar_;
    QueryPanel* queryWidget_;
};

#endif // _SEQUELJOE_MAINPANEL_H_
