/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_QUERYPANL_H_
#define _SEQUELJOE_QUERYPANL_H_

#include <QWidget>

class TableView;
class SqlModel;

class QSqlQueryModel;
class QPlainTextEdit;
class DbConnection;
class QLabel;
class QSqlQuery;

class QueryPanel: public QWidget
{
    Q_OBJECT
public:
    QueryPanel(QWidget* parent = 0);
    void setModel(SqlModel *model);

private slots:
    void executeQuery();

private:
    QPlainTextEdit* editor;
    QLabel* error;
    QLabel* status;
    TableView* results;
    SqlModel* model;
};

#endif // _SEQUELJOE_QUERYPANL_H_
