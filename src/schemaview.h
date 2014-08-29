/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SCHEMAVIEW_H_
#define _SEQUELJOE_SCHEMAVIEW_H_

#include <QWidget>

class TableView;
class QAbstractItemModel;
class QBoxLayout;
class QTreeView;
class AbstractSqlModel;

class SchemaView : public QWidget
{
    Q_OBJECT
public:
    explicit SchemaView(QWidget *parent = 0);

    struct ModelGroup {
        AbstractSqlModel* columnModel;
        AbstractSqlModel* indexModel;
    };
    void setModel(const ModelGroup& models);

signals:

public slots:
private:
    TableView* columns_;
    //TableView* indexes_;
    TableView* foreignKeys_;
    TableView* triggers_;

    TableView* indexes_;
};

#endif // _SEQUELJOE_SCHEMAVIEW_H_
