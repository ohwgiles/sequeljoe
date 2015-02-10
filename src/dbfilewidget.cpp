/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "dbfilewidget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>

DbFileWidget::DbFileWidget(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    field_ = new QLineEdit(this);
    layout->addWidget(field_);

    int sz = field_->sizeHint().height();

    browseButton_ = new QToolButton(this);
    browseButton_->setIconSize(QSize(sz-6,sz-6));
    browseButton_->setFixedSize(sz,sz);
    // hacky fine-tuning adjustment
    browseButton_->setStyleSheet("QToolButton{margin-left: -1px}");
    layout->addWidget(browseButton_);

    connect(browseButton_, SIGNAL(clicked()), this, SLOT(browseButtonClicked()));
    connect(field_, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)));

    setFile(false);
}

QSize DbFileWidget::sizeHint() const {
    return field_->sizeHint();
}

void DbFileWidget::browseButtonClicked() {
    QString file = QFileDialog::getOpenFileName(this, "Select Database File");
    if(file.isNull())
        return;
    field_->clear();
    field_->setText(file);
    changed(field_->text());
}


void DbFileWidget::setFile(bool asFile) {
    if(asFile) { // file mode
        field_->end(false);
        browseButton_->setIcon(QIcon(":browse"));
        browseButton_->show();
    } else { // name mode
        browseButton_->hide();
    }
}

void DbFileWidget::setValue(QString value) {
    field_->setText(value);
}

void DbFileWidget::textEdited(QString v) {
    emit changed(field_->text());
}
