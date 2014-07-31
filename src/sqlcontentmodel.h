/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SQLCONTENTMODEL_H_
#define _SEQUELJOE_SQLCONTENTMODEL_H_

#include <QAbstractTableModel>
#include <QVector>
#include <QEvent>

enum {
    RefreshEvent = QEvent::User
};

enum {
    FilterColumnRole = Qt::UserRole,
    FilterOperationRole,
    FilterValueRole
};

class QSqlDatabase;
class QSqlQuery;

class SqlContentModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SqlContentModel(QSqlDatabase* db, QString table, QObject *parent = 0);
    virtual ~SqlContentModel();

    static constexpr unsigned rowsPerPage() { return 1000; }

    void describe();
    void select();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);

signals:
    void pagesChanged(int,int,int);

public slots:
    void nextPage();
    void prevPage();

protected:
    bool event(QEvent *);

private:
    QSqlDatabase& db_;
    QString tableName_;

    int primaryKeyIndex_;

    unsigned int totalRecords_;
    unsigned int rowsFrom_;
    unsigned int rowsLimit_;
    bool isAdding_;
    struct ColumnHeader {
        QString name;
        QString comment;
    };
    QVector<ColumnHeader> columns_;
    QSqlQuery* query_;
    QString whereColumn_;
    QString whereOperation_;
    QString whereValue_;

};

#endif // _SEQUELJOE_SQLCONTENTMODEL_H_
