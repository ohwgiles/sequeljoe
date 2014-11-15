/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_FOREIGNKEYEDITOR_H_
#define _SEQUELJOE_FOREIGNKEYEDITOR_H_

#include <QDialog>

class QCheckBox;


class QComboBox;
class Driver;

class ForeignKeyEditor : public QDialog {
    Q_OBJECT
public:
    explicit ForeignKeyEditor(Driver *driver, QWidget* parent = 0);
    void setForeignKey(QVariant data);
    QVariant foreignKey() const;
private slots:
    void hasForeignKeyToggled(bool);
    void foreignTableChanged(QString);
private:
    QCheckBox* hasForeignRel;
    QComboBox* foreignTable;
    QComboBox* foreignColumn;
    Driver* driver;
    QString oldConstraint;
};

#endif // _SEQUELJOE_FOREIGNKEYEDITOR_H_
