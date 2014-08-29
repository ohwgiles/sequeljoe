/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SQLCONTENTMODEL_H_
#define _SEQUELJOE_SQLCONTENTMODEL_H_

#include "editablesqlmodel.h"
#include "tabledata.h"
#include "dbconnection.h"
#include <QAbstractTableModel>
#include <QVector>
#include <QEvent>


enum {
    RefreshEvent = QEvent::User
};

enum {
    FilterColumnRole = Qt::UserRole,
    FilterOperationRole,
    FilterValueRole,
    ForeignKeyTableRole,
    ForeignKeyColumnRole,
    ExpandedColumnIndexRole,
    WidgetRole
};

class DbConnection;

class QSqlDatabase;
class QSqlQuery;

class SubwidgetFactory {
public:
    virtual QWidget* createTableView(const QModelIndex& index) = 0;
};

class SqlContentModel : public EditableSqlModel
{
    Q_OBJECT
public:
    explicit SqlContentModel(DbConnection& db, QString table, QObject *parent = 0);
    virtual ~SqlContentModel();

    static constexpr unsigned rowsPerPage() { return 1000; }


    virtual void describe(const Filter &where = Filter{});
    void select();

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool deleteRows(QSet<int>);


    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override;
    virtual QModelIndex parent(const QModelIndex &child = QModelIndex{}) const;

SubwidgetFactory* subwidgetFactory_;

    virtual bool hasChildren(const QModelIndex &parent) const override;

    void setFilter(Filter& f) { where_ = f; select();}

signals:
    void pagesChanged(int,int,int);
    void selectFinished();

public slots:
    void nextPage();
    void prevPage();
    void describeComplete(TableMetadata metadata);

    void selectComplete(TableData data);

    void updateComplete(bool result, int insertId);

protected:
    bool event(QEvent *);

private:

    int updatingIndex_;
    int updatingColumn_;
    int primaryKeyIndex_;
    TableMetadata metadata_;
    QHash<int, int> expandedColumns_;
    QVector<QVariant> modifiedRow_;
    unsigned int totalRecords_;
    unsigned int rowsFrom_;
    unsigned int rowsLimit_;
    //bool isAdding_;
    //QVector<QVector<QVariant>> data_;
    Filter where_;
};

#endif // _SEQUELJOE_SQLCONTENTMODEL_H_
