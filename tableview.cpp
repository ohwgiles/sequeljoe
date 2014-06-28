/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "tableview.h"
#include <QHeaderView>

TableView::TableView(QWidget *parent) :
    QTableView(parent)
{
    // todo remove this, but resize columns to content on load complete (allowing manual column resize)
    //horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setSortIndicatorShown(true);

    //tableView->resizeRowsToContents();
    //horizontalHeader()->setStretchLastSection(true);
    verticalHeader()->setDefaultSectionSize(verticalHeader()->minimumSectionSize());
    verticalHeader()->setVisible(false);
}
