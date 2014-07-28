#ifndef SQLCONTENTMODEL_H
#define SQLCONTENTMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class QSqlDatabase;
class QSqlQuery;

class SqlContentModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SqlContentModel(QSqlDatabase* db, QString table, QObject *parent = 0);

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
    //QVector<QVector<QVariant>> data_;

signals:
    void pagesChanged(int,int,int);
public slots:
    void nextPage();
    void prevPage();
};

#endif // SQLCONTENTMODEL_H
