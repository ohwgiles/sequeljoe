/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SQLCONTENTMODEL_H_
#define _SEQUELJOE_SQLCONTENTMODEL_H_

#include "sqlmodel.h"
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
    WidgetRole
};

class DbConnection;

class QSqlDatabase;
class QSqlQuery;

struct Filter {
    QString column;
    QString operation;
    QString value;
};

class SqlContentModel : public SqlModel
{
    Q_OBJECT
public:
    explicit SqlContentModel(DbConnection& db, QString table, QObject *parent = 0);
    virtual ~SqlContentModel();

    static constexpr unsigned rowsPerPage() { return 1000; }

    void describe(const Filter &where = Filter{});
    void select();

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool deleteRows(QSet<int>);

    void setFilter(Filter& f) { where_ = f; select();}

signals:
    void pagesChanged(int,int,int);

public slots:
    void nextPage();
    void prevPage();
    void describeComplete(QVector<ColumnHeader> columns, int totalRecords, int primaryKeyIndex);

    void selectComplete(QVector<QVector<QVariant> > data);

    void updateComplete(bool result, int insertId);

protected:
    bool event(QEvent *);

private:
    DbConnection& db_;
    int updatingIndex_;
    int updatingColumn_;
    int primaryKeyIndex_;
    QVector<ColumnHeader> columns_;

    QVector<QVariant> modifiedRow_;
    unsigned int totalRecords_;
    unsigned int rowsFrom_;
    unsigned int rowsLimit_;
    //bool isAdding_;
    //QVector<QVector<QVariant>> data_;
    Filter where_;
};

#endif // _SEQUELJOE_SQLCONTENTMODEL_H_
