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
    field_ = new QLineEdit(this);
    layout->addWidget(field_);

    int sz = field_->sizeHint().height();

    keyButton_ = new QToolButton(this);
    keyButton_->setIconSize(QSize(sz-6,sz-6));
    keyButton_->setFixedSize(sz,sz);
    keyButton_->setStyleSheet("QToolButton{margin-left: -1px}");
    layout->addWidget(keyButton_);

    connect(keyButton_, SIGNAL(clicked()), this, SLOT(keyButtonClicked()));
    connect(field_, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)));

    toggleMode(false);
}

QSize PassKeyWidget::sizeHint() const {
    return field_->sizeHint();
}

void PassKeyWidget::keyButtonClicked() {
    bool isKeyMode = field_->isReadOnly();
    if(isKeyMode) {
        field_->clear();
    } else {
        QString file = QFileDialog::getOpenFileName(this, "Select Key");
        if(file.isNull()) return;
        field_->clear();
        field_->setText(file);
    }
    toggleMode(!isKeyMode);
}

void PassKeyWidget::toggleMode(bool key, bool doEmit) {
    if(key) { // key mode
        field_->setEchoMode(QLineEdit::Normal);
        field_->setReadOnly(true);
        field_->end(false);
        keyButton_->setIcon(QIcon(":remove"));
        if(doEmit)
            emit changed(true, field_->text());
    } else { // password mode
        field_->setEchoMode(QLineEdit::Password);
        field_->setReadOnly(false);
        keyButton_->setIcon(QIcon(":key"));
        if(doEmit) emit changed(false, field_->text());
    }
}

void PassKeyWidget::setValue(bool key, QString value) {
    field_->setText(value);
    toggleMode(key, false);
}

void PassKeyWidget::textEdited(QString v) {
    emit changed(false, field_->text());
}
