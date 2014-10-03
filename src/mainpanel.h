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
#include <QHash>

class ConnectionWidget;
class FilteredPagedTableView;
class SchemaView;
class DbConnection;
class TableList;
class QueryLog;
class TableModel;
class SqlSchemaModel;

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

protected:
    void resizeEvent(QResizeEvent *) override;

private slots:
    void addTable();
    void deleteTable();
    void refreshTables();

    void databaseConnected();
    void connectionFailed(QString reason);
    void confirmUnknownHost(QString fingerprint, bool* ok);

    void tableListChanged();

private:
    void updateContentModel(QString tableName);
    void updateSchemaModel(QString tableName);

    DbConnection* db;
    QThread* backgroundWorker;
    QWidget* loadingOverlay;

    QHash<QString, TableModel*> contentModels;
    struct SchemaModels {
        QAbstractItemModel* index;
        QAbstractItemModel* schema;
    };
    QHash<QString, SchemaModels> schemaModels;

    ConnectionWidget* settingsPanel;
    QSplitter* splitTableChooser;
    QSplitter* splitLogViewer;
    TableList* tableChooser;
    FilteredPagedTableView* contentView;
    SchemaView* schemaView;
    ViewToolBar* toolbar;
    QueryPanel* queryWidget;
    QueryLog* queryLog;
};

#endif // _SEQUELJOE_MAINPANEL_H_
