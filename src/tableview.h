/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_TABLEVIEW_H_
#define _SEQUELJOE_TABLEVIEW_H_

#include <QTreeView>

class QMenu;
class QAction;

class TableView : public QTreeView
{
    Q_OBJECT
public:
    explicit TableView(QWidget *parent = 0);

public slots:
    void openMenu(QPoint);
    void handleSetNull();
    void handleDeleteRow();
    void handleAddRow();
    void showLoadingOverlay(bool show);

    void setModel(QAbstractItemModel *model);
    void adjustColumnSizes();

protected slots:
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void toggleForeignTable(const QModelIndex&);
    void handleModelReset();

private:
    virtual QWidget* createChildTable(const QModelIndex& index);

    QMenu* contextMenu;
    QAction* setNullAction;
    QAction* deleteRowAction;
    QAction* addRowAction;
    QWidget* loadingOverlay;
    QHash<int,QHash<int,QWidget*>> foreignTableViews;
};

#endif // _SEQUELJOE_TABLEVIEW_H_
