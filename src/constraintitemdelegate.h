#ifndef CONSTRAINTITEMDELEGATE_H
#define CONSTRAINTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QApplication>
#include <QStyle>
#include <QPainter>
#include <QDebug>


class ConstraintItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
ConstraintItemDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
signals:
    void constraintActivated(const QModelIndex&);
};

#endif // CONSTRAINTITEMDELEGATE_H
