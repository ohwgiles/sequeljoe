/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_CONSTRAINTEDITOR_H_
#define _SEQUELJOE_CONSTRAINTEDITOR_H_

#include <QDialog>
#include "tabledata.h"

class QComboBox;
class QLineEdit;
class DbConnection;
class QFormLayout;
class QListWidget;
class ConstraintEditor : public QDialog {
    Q_OBJECT
public:
    enum { Deleted = (Accepted + 1) }; // c.f. QDialog::DialogCode
    explicit ConstraintEditor(QString currentTable, DbConnection *driver, QWidget* parent = 0);
    virtual ~ConstraintEditor() {}
    void setConstraint(Constraint c);
    Constraint constraint() const;
private slots:
    void constraintTypeChanged(int);
    void foreignTableChanged(QString);
private:
    QLineEdit* constraintName;
    QComboBox* constraintType;
    QComboBox* foreignTable;
    QComboBox* foreignColumn;
    DbConnection* driver;
    QString currentTable;
    QString sourceColumn;
    QListWidget* uniqueColumns;
    QFormLayout* formLayout;
};

#endif // _SEQUELJOE_CONSTRAINTEDITOR_H_
