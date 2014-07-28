#ifndef SQLCONTENTMODEL_H
#define SQLCONTENTMODEL_H

#include <QAbstractItemModel>

class SqlContentModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SqlContentModel(QObject *parent = 0);

signals:

public slots:

};

#endif // SQLCONTENTMODEL_H
