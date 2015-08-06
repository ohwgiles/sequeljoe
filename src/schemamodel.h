/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SQLSCHEMAMODEL_H_
#define _SEQUELJOE_SQLSCHEMAMODEL_H_

#include "sqlmodel.h"
#include "tabledata.h"
#include "foreignkey.h"

class DbConnection;
class SchemaConstraintsProxyModel;

class SqlSchemaModel : public SqlModel
{
    Q_OBJECT
public:
    explicit SqlSchemaModel(DbConnection &db, QString tableName, QObject *parent = 0);
    virtual ~SqlSchemaModel() {}

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool deleteRows(QSet<int>) override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void select() override;

    QAbstractItemModel* constraintsModel() const;

//    struct ConstraintHelper {
//        Constraint constraint;
//        int sequence;
//        QString sourceColumn;
//    };

signals:
    void schemaModified(QString);


protected:
    Schema schema;
    virtual bool columnIsBoolType(int col) const override;

public slots:
    void saveConstraint(Constraint c);
    void removeConstraint(Constraint c);
protected slots:
    bool submit() override;
    virtual void selectComplete(int nRows) override;

private:
    QString schemaQuery(const std::array<QVariant, SCHEMA_NUM_FIELDS> &def);
    QString originalColumnName;
    QString tableName;
    SchemaConstraintsProxyModel* constraintsProxy;
};

#endif // _SEQUELJOE_SQLSCHEMAMODEL_H_
