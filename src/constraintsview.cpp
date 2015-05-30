#include "constraintsview.h"
#include "tabledata.h"
#include "constrainteditor.h"

#include "constraintitemdelegate.h"
#include "roles.h"

ConstraintsView::ConstraintsView(QWidget *parent) : QListView(parent) {
    setDragEnabled(false);
    setUniformItemSizes(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Preferred);
    setFrameStyle(0);
    viewport()->setBackgroundRole(QPalette::Background);
    ConstraintItemDelegate* cid = new ConstraintItemDelegate(this);
    connect(cid, &ConstraintItemDelegate::constraintActivated, [&](const QModelIndex& idx){
        editConstraint(idx.data(Qt::EditRole).value<Constraint>(), idx);
    });
    setItemDelegate(cid);
}

void ConstraintsView::editConstraint(Constraint c, const QModelIndex& idx) {
    // yech
    ConstraintEditor ce(model()->data(QModelIndex{},TableNameRole).toString(),db);
    ce.setConstraint(c);
    int result = ce.exec();
    if(result == QDialog::Accepted)
        model()->setData(idx, QVariant::fromValue(ce.constraint()));
    else if(result == ConstraintEditor::Deleted)
        model()->setData(idx, QVariant());
}
ConstraintsView::~ConstraintsView()
{

}

QSize ConstraintsView::sizeHint() const {
    if(model()) {
        int rowCount = model()->rowCount();
        return QSize(200, sizeHintForRow(0) * rowCount );
    } else
        return viewport()->sizeHint();
}

