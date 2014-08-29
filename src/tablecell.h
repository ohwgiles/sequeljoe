/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_TABLECELL_H_
#define _SEQUELJOE_TABLECELL_H_

#include <QStyledItemDelegate>

class TableCell : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TableCell(QObject *parent = 0);


    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
public slots:
    void handleCollapse(const QModelIndex& index);
    void handleExpand(const QModelIndex& index);
signals:
    void goToForeignEntry(const QModelIndex& index);
protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index);
private:
    QRect expandWidgetRect(QRect cellRect) const;
};

#endif // _SEQUELJOE_TABLECELL_H_
