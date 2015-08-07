/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SQLMODEL_H_
#define _SEQUELJOE_SQLMODEL_H_

#include "tabledata.h"
#include "dbconnection.h"
#include "roles.h"

#include <QAbstractTableModel>
#include <QVector>
#include <QEvent>
#include <QSet>
#include <QSqlQuery>

class DbConnection;

class QSqlDatabase;
class QSqlQuery;

enum {
    RefreshEvent = QEvent::User
};

class SqlModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit SqlModel(DbConnection &db, QObject *parent = 0);
    virtual ~SqlModel() {}

    int rowsPerPage() const { return rowsLimit; }
    void signalPagination() const { emit pagesChanged(rowsFrom, rowCount(), totalRecords); }
    void setQuery(QString q) { query = q; }
    virtual void select();
    virtual bool deleteRows(QSet<int>) { return false; }

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override;
    virtual QModelIndex parent(const QModelIndex &child = QModelIndex{}) const override;
    virtual bool hasChildren(const QModelIndex &parent) const override;

    bool insertRows(int row, int count, const QModelIndex &parent) override;
    // hack to fetch for ForeignKeyEditor
    DbConnection* driver() const { return &db; }

    void setRowsPerPage(int r, bool refresh = true) { rowsLimit = r; if(refresh) select(); }
signals:
    void pagesChanged(int,int,int) const;
    void selectFinished();

public slots:
    void firstPage();
    void nextPage();
    void prevPage();
    void lastPage();

protected slots:
    virtual void selectComplete(int nRows);
    void updateComplete(int rowsAffected, int insertId);
    void deleteComplete(int rowsAffected, int);
protected:
    virtual bool event(QEvent *) override;
    virtual QString prepareQuery() const { return query; }
    virtual bool columnIsBoolType(int col) const;

protected:
    bool isAdding() const { return (updatingRow != -1); }

    DbConnection& db;
    QString query;

    bool dataSafe;
    mutable QSqlQuery res;
    TableMetadata metadata;
    QHash<int, int> expandedColumns;

    int updatingRow;
    QHash<int, QVariant> currentRowModifications;

    int numRows;
    unsigned int totalRecords;
    int rowsFrom;
    unsigned int rowsLimit;
};

#endif // _SEQUELJOE_SQLMODEL_H_
