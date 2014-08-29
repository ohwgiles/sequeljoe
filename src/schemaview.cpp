/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "schemaview.h"

#include "tableview.h"
#include "abstractsqlmodel.h"
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

    columns_ = new TableView(this);
    splitter->addWidget(columns_);

    indexes_ = new TableView(this);
    //indexes_->setUniformRowHeights(true);
    splitter->addWidget(indexes_);

    layout->addWidget(splitter);
//    layout->addWidget(indexes_);
}

void SchemaView::setModel(const ModelGroup &models) {
    columns_->setModel(models.columnModel);
    indexes_->setModel(models.indexModel);
//    auto fn = [=](){
//        if(models.indexModel) {
//        for(int i = 0; i < models.indexModel->rowCount(); ++i)
//        indexes_->setFirstColumnSpanned(i, QModelIndex{}, true);//models.indexModel->index(0,0)
//        indexes_->expandAll();
//        }
//    };
//    fn();
//    connect(models.indexModel, &AbstractSqlModel::modelReset, fn);

//    QLayoutItem* i;
//    while((i = indexes_->takeAt(0))) {
//        delete i->widget();
//        delete i;
//    }
//    indexes_->addWidget(new QLabel("test"));

}
