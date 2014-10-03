/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "passkeywidget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>

PassKeyWidget::PassKeyWidget(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    field = new QLineEdit(this);
    layout->addWidget(field);

    int sz = field->sizeHint().height();

    keyButton = new QToolButton(this);
    keyButton->setIconSize(QSize(sz-6,sz-6));
    keyButton->setFixedSize(sz,sz);
    keyButton->setStyleSheet("QToolButton{margin-left: -1px}");
    layout->addWidget(keyButton);

    connect(keyButton, SIGNAL(clicked()), this, SLOT(keyButtonClicked()));
    connect(field, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)));

    toggleMode(false);
}

QSize PassKeyWidget::sizeHint() const {
    return field->sizeHint();
}

void PassKeyWidget::keyButtonClicked() {
    bool isKeyMode = field->isReadOnly();
    if(isKeyMode) {
        field->clear();
    } else {
        QString file = QFileDialog::getOpenFileName(this, "Select Key");
        if(file.isNull()) return;
        field->clear();
        field->setText(file);
    }
    toggleMode(!isKeyMode);
}

void PassKeyWidget::toggleMode(bool key, bool doEmit) {
    if(key) { // key mode
        field->setEchoMode(QLineEdit::Normal);
        field->setReadOnly(true);
        field->end(false);
        keyButton->setIcon(QIcon(":remove"));
        if(doEmit)
            emit changed(true, field->text());
    } else { // password mode
        field->setEchoMode(QLineEdit::Password);
        field->setReadOnly(false);
        keyButton->setIcon(QIcon(":key"));
        if(doEmit) emit changed(false, field->text());
    }
}

void PassKeyWidget::setValue(bool key, QString value) {
    field->setText(value);
    toggleMode(key, false);
}

void PassKeyWidget::textEdited(QString v) {
    emit changed(false, field->text());
}
