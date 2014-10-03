/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_QUERYLOG_H_
#define _SEQUELJOE_QUERYLOG_H_

#include <QTableWidget>

class QSqlQuery;

class QueryLog : public QTableWidget
{
    Q_OBJECT
public:
    explicit QueryLog(QWidget *parent = 0);

public slots:
    void logQuery(QString query, QString status);
};

#endif // _SEQUELJOE_QUERYLOG_H_
