/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "constrainteditor.h"
#include "foreignkey.h"
#include "dbconnection.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QListWidget>

ConstraintEditor::ConstraintEditor(QString currentTable, DbConnection *driver, QWidget *parent) :
    QDialog(parent),
    driver(driver),
    currentTable(currentTable),
    formLayout(0)
{
    setModal(true);
    QBoxLayout* l = new QVBoxLayout(this);
    setWindowTitle("Edit Constraint");

    constraintName = new QLineEdit(this);

    constraintType = new QComboBox(this);
    constraintType->addItem("Foreign Key");
    constraintType->addItem("Unique");
    connect(constraintType,
        (void (QComboBox::*)(int)) &QComboBox::currentIndexChanged,
        this, &ConstraintEditor::constraintTypeChanged);

    // foreign key
    foreignTable = new QComboBox(this);
    connect(foreignTable, SIGNAL(currentTextChanged(QString)), this, SLOT(foreignTableChanged(QString)));
    foreignColumn = new QComboBox(this);

    // unique columns
    uniqueColumns = new QListWidget(this);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel, this);
    QPushButton* remove = new QPushButton("Remove Constraint", this);
    connect(remove, &QPushButton::clicked, [this]{
        done(Deleted);
    });
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    buttonBox->addButton(remove, QDialogButtonBox::DestructiveRole);
    l->addWidget(buttonBox);
    constraintTypeChanged(ConstraintDetail::CONSTRAINT_FOREIGNKEY);
}

void ConstraintEditor::constraintTypeChanged(int v) {
    // QTBUG-6864
    if(formLayout) {
        delete formLayout->labelForField(constraintName);
        delete formLayout->labelForField(constraintType);
        delete formLayout->labelForField(foreignTable);
        delete formLayout->labelForField(foreignColumn);
        delete formLayout->labelForField(uniqueColumns);
        delete formLayout;
    }
    formLayout = new QFormLayout(this);

    formLayout->addRow("Name", constraintName);
    formLayout->addRow("Type", constraintType);
    foreignTable->setVisible(v == ConstraintDetail::CONSTRAINT_FOREIGNKEY);
    foreignColumn->setVisible(v == ConstraintDetail::CONSTRAINT_FOREIGNKEY);
    uniqueColumns->setVisible(v == ConstraintDetail::CONSTRAINT_UNIQUE);
    if(v == ConstraintDetail::CONSTRAINT_FOREIGNKEY) {
        formLayout->addRow("Table", foreignTable);
        formLayout->addRow("Column", foreignColumn);
    } else {
        uniqueColumns->clear();
        uniqueColumns->addItems(driver->columnNames(currentTable));
        for(int i = 0; i < uniqueColumns->count(); ++i) {
            QListWidgetItem* item = uniqueColumns->item(i);
            item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
            item->setCheckState(Qt::Unchecked);
        }
//        for(QString s : driver->columnNames(currentTable)) {
//            QListWidgetItem* item = new QListWidgetItem(s);
//            item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
//            item->setCheckState(Qt::Unchecked);
//            uniqueColumns->addItem(item);
//        }
        formLayout->addRow("Columns", uniqueColumns);
    }
    qobject_cast<QVBoxLayout*>(layout())->insertLayout(0, formLayout);
}

void ConstraintEditor::foreignTableChanged(QString s) {
    if(!s.isEmpty()) {
        foreignColumn->clear();
        foreignColumn->addItems(driver->columnNames(s));
    }
}

void ConstraintEditor::setConstraint(Constraint c) {
    constraintName->setText(c.name);
    sourceColumn = c.detail.fk.column;
     // BRITTLE, works since order added to combo box matches enum values
    constraintType->setCurrentIndex(c.detail.type);
    if(c.detail.type == ConstraintDetail::CONSTRAINT_FOREIGNKEY) {
        foreignTable->addItems(driver->tables());
        foreignTable->setCurrentText(c.detail.fk.refTable);
        foreignColumn->addItems(driver->columnNames(foreignTable->currentText()));
        foreignColumn->setCurrentText(c.detail.fk.refColumn);
    } else {
        for(int i = 0; i < uniqueColumns->count(); ++i) {
            QListWidgetItem* item = uniqueColumns->item(i);
            item->setCheckState(c.detail.cols.contains(item->text()) ? Qt::Checked : Qt::Unchecked);
        }
    }
}

Constraint ConstraintEditor::constraint() const {
    Constraint c;
    c.name = constraintName->text();
    // todo: generate name if it's empty
    // BRITTLE, see above
    *((int*)&c.detail.type) = constraintType->currentIndex(); // take THAT, compiler!
    if(constraintType->currentIndex() == ConstraintDetail::CONSTRAINT_FOREIGNKEY) {
        c.detail.type = ConstraintDetail::CONSTRAINT_FOREIGNKEY;
        c.detail.fk.column = sourceColumn;
        c.detail.fk.refTable = foreignTable->currentText();
        c.detail.fk.refColumn = foreignColumn->currentText();
    } else {
        // todo
    }
    return c;
}
