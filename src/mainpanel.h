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
#include "sqlcontentmodel.h" // for filter, move to own class
class ConnectionWidget;
class FilteredPagedTableView;
class DbConnection;
class TableList;
class QueryLog;

class QTableView;
class QSqlTableModel;
class QListWidgetItem;
class QListView;
class QLineEdit;
class QSortFilterProxyModel;
class QueryPanel;
class QSplitter;
class QThread;

class MainPanel : public QWidget
{
    Q_OBJECT
public:
    explicit MainPanel(QWidget *parent = 0);
    virtual ~MainPanel();
    void loadSettings(QListWidgetItem* item);

signals:
    void nameChanged(QWidget*,QString);

public slots:
    void toggleEditSettings(bool showSettings = true);

    void openConnection(QString name);
    void tableChanged(QString name);

    void changeSort(int, Qt::SortOrder);

    void openPanel(ViewToolBar::Panel);

    void disconnectDb();

    void dbChanged(QString);

private slots:
    void addTable();
    void deleteTable();
    void refreshTables();
    void jumpToQuery(QString table, QString column, QVariant value);

    void databaseConnected();
    void tableListChanged();

private:
    void updateContentModel(QString tableName);
    void updateSchemaModel(QString tableName);

    DbConnection* db_;
    ConnectionWidget* settings_;
    QSplitter* contentSchemaSplit_;
    QSplitter* logSplit_;
    TableList* tableChooser_;
    FilteredPagedTableView* content_;
    QTableView* structure_;
    ViewToolBar* toolbar_;
    QueryPanel* queryWidget_;
    QueryLog* queryLog_;
    QThread* backgroundWorker_;
    Filter jumpToTableFilter_;
};

#endif // _SEQUELJOE_MAINPANEL_H_
