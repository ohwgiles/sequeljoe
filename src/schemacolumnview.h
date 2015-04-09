#ifndef SCHEMACOLUMNVIEW_H
#define SCHEMACOLUMNVIEW_H

#include "tableview.h"

class SchemaColumnView : public TableView {
    Q_OBJECT
public:
    SchemaColumnView(QWidget* parent = 0);
    virtual ~SchemaColumnView();
signals:
    void addForeignKey(QString column);
};

#endif // SCHEMACOLUMNVIEW_H
