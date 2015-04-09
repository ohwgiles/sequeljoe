#ifndef CONSTRAINTSVIEW_H
#define CONSTRAINTSVIEW_H

#include <QListView>
#include <QWheelEvent>
#include "tabledata.h"
class DbConnection;
class ConstraintsView : public QListView
{
    Q_OBJECT
public:
    ConstraintsView(QWidget* parent = 0);
    ~ConstraintsView();
//    virtual QSize viewportSizeHint() const override {
//        return QSize(200,10);
//    }
    virtual QSize sizeHint() const override;

    virtual QSize minimumSizeHint() const override { return sizeHint(); }
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override {}
    void wheelEvent(QWheelEvent* event) override {
        event->ignore();
    }
    // todo encapsulate
    DbConnection* db;
public slots:
    void editConstraint(Constraint c, const QModelIndex& idx = QModelIndex{});
private:
};

#endif // CONSTRAINTSVIEW_H
