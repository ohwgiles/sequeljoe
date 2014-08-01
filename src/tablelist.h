/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_TABLELIST_H_
#define _SEQUELJOE_TABLELIST_H_

#include <QWidget>

class QListView;
class QLineEdit;
class QStringListModel;

class TableList : public QWidget
{
    Q_OBJECT
public:
    explicit TableList(QWidget *parent = 0);
    void setTableNames(QStringList names);
    QString selectedTable() const;
    void setCurrentTable(QString name);

signals:
    void tableSelected(QString name);
    void addButtonClicked();
    void delButtonClicked();
    void refreshButtonClicked();

private slots:
    void filterTextChanged(QString text);
    void selectionChanged(QModelIndex index);

private:
    QListView* tables_;
    QLineEdit* filterInput_;
    QStringListModel* tableItems_;
};

#endif // _SEQUELJOE_TABLELIST_H_
