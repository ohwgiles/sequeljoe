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
class QueryModel;


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
    void setDb(DbConnection* db);

private slots:
    void executeQuery();

private:
    QPlainTextEdit* editor_;
    QLabel* error_;
    QLabel* status_;
    TableView* results_;
    QueryModel* model_;
    QSqlQuery* query_;
};

#endif // _SEQUELJOE_QUERYPANL_H_
