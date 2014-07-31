/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_MAINPANEL_H_
#define _SEQUELJOE_MAINPANEL_H_

#include "viewtoolbar.h"
#include <QWidget>
#include <QModelIndex>

class ConnectionWidget;
class FilteredPagedTableView;
class DbConnection;
class TableList;

class QTableView;
class QSqlTableModel;
class QListWidgetItem;
class QListView;
class QLineEdit;
class QSortFilterProxyModel;
class QueryPanel;
class QSplitter;

class MainPanel : public QWidget
{
    Q_OBJECT
public:
    explicit MainPanel(QWidget *parent = 0);
    void loadSettings(QListWidgetItem* item);

signals:
    void nameChanged(QWidget*,QString);

public slots:
    void toggleEditSettings(bool showSettings = true);
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
    QSplitter* contentSchemaSplit_;
    QSplitter* logSplit_;
    TableList* tableChooser_;
    FilteredPagedTableView* content_;
    QTableView* structure_;
    ViewToolBar* toolbar_;
    QueryPanel* queryWidget_;
};

#endif // _SEQUELJOE_MAINPANEL_H_
