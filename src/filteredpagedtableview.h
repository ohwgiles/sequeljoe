/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_FILTEREDPAGEDTABLEVIEW_H_
#define _SEQUELJOE_FILTEREDPAGEDTABLEVIEW_H_

#include "tableview.h"

class QAbstractButton;
class QLabel;
class QComboBox;
class QLineEdit;

class FilteredPagedTableView : public QWidget {
    Q_OBJECT
public:
    explicit FilteredPagedTableView(QWidget *parent = 0);

    void setModel(QAbstractItemModel* m);

    QAbstractItemModel* model() const { return table_->model(); }
    QHeaderView* horizontalHeader() const { return table_->horizontalHeader(); }

protected:
    QStringList filterOperations() const;

private slots:
    void updatePagination(int,int,int);
    void clearFilter();
    void runFilter();
    void refreshModel();

private:
    QAbstractButton* b;
    QTableView* table_;
    QComboBox* filterColumns_;
    QComboBox* filterOperation_;
    QLineEdit* filterText_;
    QAbstractButton* filterRun_;
    QAbstractButton* filterClear_;
    QAbstractButton* prev_;
    QAbstractButton* next_;
    QLabel* pageNum_;
};

#endif // _SEQUELJOE_FILTEREDPAGEDTABLEVIEW_H_
