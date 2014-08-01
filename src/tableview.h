/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_TABLEVIEW_H_
#define _SEQUELJOE_TABLEVIEW_H_

#include <QTableView>

class QMenu;
class QAction;

class TableView : public QTableView
{
    Q_OBJECT
public:
    explicit TableView(QWidget *parent = 0);

signals:
    void foreignQuery(QString table, QString column, QVariant value);

public slots:
    void openMenu(QPoint);
    void handleSetNull();
    void handleDeleteRow();
    void handleAddRow();

private slots:
    void handleRequestForeignKey(const QModelIndex&);

private:
    QMenu* ctxMenu_;
    QAction* nullAction_;
    QAction* deleteRowAction_;
    QAction* addRowAction_;
};

#endif // _SEQUELJOE_TABLEVIEW_H_
