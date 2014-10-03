/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "schemaview.h"

#include "tableview.h"
#include "sqlmodel.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QTreeView>
#include <QLabel>

SchemaView::SchemaView(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    QSplitter* splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);

    columns = new TableView(this);
    columns->setUniformRowHeights(true);
    splitter->addWidget(columns);

    indexes = new TableView(this);
    indexes->setUniformRowHeights(true);
    splitter->addWidget(indexes);

    layout->addWidget(splitter);
}

void SchemaView::setModels(QAbstractItemModel* schema, QAbstractItemModel* index) {
    columns->setModel(schema);
    indexes->setModel(index);
}
