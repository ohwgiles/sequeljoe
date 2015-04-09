#include "schemacolumnview.h"
#include <QAction>
#include "tabledata.h"
#include <QMenu>
SchemaColumnView::SchemaColumnView(QWidget* parent) : TableView(parent) {
    setUniformRowHeights(true);
    QAction* a = new QAction("Add Foreign Key Relationship", contextMenu);
    connect(a, &QAction::triggered, [&]{
        QModelIndex idx = currentIndex();
        QString column = idx.sibling(idx.row(),SCHEMA_NAME).data(Qt::EditRole).toString();

        emit addForeignKey(column); });
    contextMenu->addAction(a);

}


SchemaColumnView::~SchemaColumnView()
{

}

