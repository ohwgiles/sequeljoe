#ifndef TABLECELL_H
#define TABLECELL_H

#include <QItemDelegate>

class TableCell : public QItemDelegate
{
    Q_OBJECT
public:
    explicit TableCell(QObject *parent = 0);
virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
signals:

public slots:

};

#endif // TABLECELL_H
