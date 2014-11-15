/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "foreignkeyeditor.h"
#include "foreignkey.h"
#include "driver.h"

#include <QVBoxLayout>
#include <QCheckBox>

#include <QComboBox>
ForeignKeyEditor::ForeignKeyEditor(Driver *driver, QWidget *parent) :
    QDialog(parent),
    driver(driver)
{
    setModal(true);
    QBoxLayout* l = new QVBoxLayout(this);
    setWindowTitle("Edit Foreign Key");
    hasForeignRel = new QCheckBox("Has Foreign Key Relationship",this);
    connect(hasForeignRel, SIGNAL(toggled(bool)), this, SLOT(hasForeignKeyToggled(bool)));
    l->addWidget(hasForeignRel);

    foreignTable = new QComboBox(this);
    foreignColumn = new QComboBox(this);
    l->addWidget(foreignTable);
    l->addWidget(foreignColumn);
}

void ForeignKeyEditor::hasForeignKeyToggled(bool v) {
    foreignTable->setEnabled(v);
    foreignColumn->setEnabled(v);
}

void ForeignKeyEditor::foreignTableChanged(QString s) {
    foreignColumn->clear();
    foreignColumn->addItems(QStringList::fromVector(driver->metadata(s).columnNames));
}

void ForeignKeyEditor::setForeignKey(QVariant data) {
    ForeignKey fk = data.value<ForeignKey>();
    oldConstraint = fk.constraint;
    hasForeignRel->setChecked(!fk.column.isNull());
    hasForeignKeyToggled(!fk.column.isNull());
    foreignTable->addItems(driver->tableNames());
    foreignTable->setCurrentText(fk.table);
    foreignColumn->addItems(QStringList::fromVector(driver->metadata(foreignTable->currentText()).columnNames));
    foreignColumn->setCurrentText(fk.column);
    connect(foreignTable, SIGNAL(currentTextChanged(QString)), this, SLOT(foreignTableChanged(QString)));
}

QVariant ForeignKeyEditor::foreignKey() const {
    if(hasForeignRel->isChecked()) {
        ForeignKey fk;
        fk.table = foreignTable->currentText();
        fk.column = foreignColumn->currentText();
        fk.constraint = oldConstraint;
        return QVariant::fromValue(fk);
    } else
        return QVariant();
}
