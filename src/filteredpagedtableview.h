/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_FILTEREDPAGEDTABLEVIEW_H_
#define _SEQUELJOE_FILTEREDPAGEDTABLEVIEW_H_

#include <QWidget>
#include <QAbstractItemModel>

class TableView;

class QAbstractButton;
class QLabel;
class QComboBox;
class QLineEdit;

class FilteredPagedTableView : public QWidget {
    Q_OBJECT
public:
    explicit FilteredPagedTableView(QWidget *parent = 0);

    void setModel(QAbstractItemModel* m);
    QAbstractItemModel* model() const;

    void setFilter(QString column, QString operation, QVariant vaulue);

private slots:
    void updatePagination(int,int,int);
    void populateFilter();
    void clearFilter();
    void runFilter();
    void refreshModel();

private:
    QStringList filterOperations() const;

    TableView* table;
    QComboBox* filterColumns;
    QComboBox* filterOperation;
    QLineEdit* filterText;
    QAbstractButton* filterRun;
    QAbstractButton* filterClear;
    QAbstractButton* prev;
    QAbstractButton* next;
    QLabel* pageNum;
};

#endif // _SEQUELJOE_FILTEREDPAGEDTABLEVIEW_H_
