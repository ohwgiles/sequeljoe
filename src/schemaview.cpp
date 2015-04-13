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
#include <QListView>
#include <QMenu>

#include "schemamodel.h"
#include "constraintsview.h"
#include "schemacolumnview.h"

SchemaView::SchemaView(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    columns = new SchemaColumnView(this);
    constraints = new ConstraintsView(this);
    connect(columns, &SchemaColumnView::addForeignKey, [&](QString column){
        Constraint c;
        c.detail.type = ConstraintDetail::CONSTRAINT_FOREIGNKEY;
        c.detail.fk.column = column;
        constraints->editConstraint(c);
    });
    layout->addWidget(columns);
    layout->addWidget(constraints);
}

void SchemaView::setModel(SqlSchemaModel* schema) {
    columns->setModel(schema);
    disconnect(constraints->model());
    if(schema) {
        constraints->db = schema->driver();

        constraints->setModel(schema->constraintsModel());
        connect(constraints->model(), &QAbstractItemModel::layoutChanged, [&](){
            constraints->updateGeometry();
        });
        constraints->updateGeometry();
    } else {
        constraints->db = nullptr;
        constraints->setModel(nullptr);
    }
}

