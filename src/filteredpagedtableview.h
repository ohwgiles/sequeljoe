#ifndef FILTEREDPAGEDTABLEVIEW_H
#define FILTEREDPAGEDTABLEVIEW_H

#include "tableview.h"
class QAbstractButton;
class QLabel;
class QComboBox;
class QLineEdit;

class FilteredPagedTableView : public QWidget
{
    Q_OBJECT
public:
    explicit FilteredPagedTableView(QWidget *parent = 0);

    void setModel(QAbstractItemModel* m);
    QAbstractItemModel* model() const { return table_->model(); }
    QHeaderView* horizontalHeader() const { return table_->horizontalHeader(); }

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

signals:

public slots:

private slots:
    void updatePagination(int,int,int);
    void clearFilter();
    void runFilter();
    void refreshModel();
};

#endif // FILTEREDPAGEDTABLEVIEW_H
