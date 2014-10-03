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

    static constexpr unsigned rowsPerPage() { return 1000; }

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

signals:
    void pagesChanged(int,int,int);
    void selectFinished();

public slots:
    void nextPage();
    void prevPage();

protected slots:
    virtual void selectComplete(TableData data);
    void updateComplete(bool result, int insertId);

protected:
    virtual bool event(QEvent *) override;
    virtual QString prepareQuery() const { return query; }
    virtual bool columnIsBoolType(int col) const;

protected:
    bool isAdding() const { return (updatingRow != -1); }

    DbConnection& db;
    QString query;

    bool dataSafe;
    TableData content;
    TableMetadata metadata;
    int primaryKeyColumn;
    QHash<int, int> expandedColumns;

    int updatingRow;
    QHash<int, QVariant> currentRowModifications;

    unsigned int totalRecords;
    unsigned int rowsFrom;
    unsigned int rowsLimit;
};

#endif // _SEQUELJOE_SQLMODEL_H_
