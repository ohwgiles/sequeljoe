#ifndef FILTEREDPAGEDTABLEVIEW_H
#define FILTEREDPAGEDTABLEVIEW_H

#include "tableview.h"
class QPushButton;
class QLabel;
class FilteredPagedTableView : public QWidget
{
    Q_OBJECT
public:
    explicit FilteredPagedTableView(QWidget *parent = 0);

    void setModel(QAbstractItemModel* m);
    QAbstractItemModel* model() const { return table_->model(); }
    QHeaderView* horizontalHeader() const { return table_->horizontalHeader(); }

private:
    QPushButton* b;
    QTableView* table_;

    QPushButton* prev_;
    QPushButton* next_;
    QLabel* pageNum_;

signals:

public slots:

private slots:
    void updatePagination(int,int,int);
};

#endif // FILTEREDPAGEDTABLEVIEW_H
