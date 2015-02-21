/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "querylog.h"

#include <QSqlQuery>
#include <QHeaderView>
#include <QSqlError>
#include <QDateTime>

QueryLog::QueryLog(QWidget *parent) :
    QTableWidget(parent)
{
    setColumnCount(1);
    setAlternatingRowColors(true);
    setEditTriggers(QTableWidget::NoEditTriggers);
    setSelectionMode(QAbstractItemView::NoSelection);
    setShowGrid(false);

    verticalHeader()->setDefaultSectionSize(verticalHeader()->minimumSectionSize());
    verticalHeader()->setVisible(false);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setVisible(false);
}


void QueryLog::logQuery(QString query, QString statusString) {
    QTableWidgetItem* item = new QTableWidgetItem(QDateTime::currentDateTime().toString(Qt::ISODate) + ": " + query.replace('\n', ' '));
    QTableWidgetItem* status = new QTableWidgetItem(statusString);
    int rows = rowCount();
    setRowCount(rows+2);
    setItem(rows, 0, item);
    setItem(rows+1,0, status);
    scrollToBottom();
}
