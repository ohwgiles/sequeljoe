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

class SubwidgetFactory {
public:
    virtual QWidget* createTableView(const QModelIndex& index) = 0;
};


class TableCell : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TableCell(SubwidgetFactory& subWidgetFactory, QObject *parent = 0);

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

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

    SubwidgetFactory& subwidgetFactory_;

};

#endif // _SEQUELJOE_TABLECELL_H_
