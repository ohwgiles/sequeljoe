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
    browseButton_->setStyleSheet("QToolButton{margin-left: -1px}");
    layout->addWidget(browseButton_);

    connect(browseButton_, SIGNAL(clicked()), this, SLOT(browseButtonClicked()));
    connect(field_, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)));

    toggleMode(false);
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
    changed(true, field_->text());
}

void DbFileWidget::toggleMode(bool asFile, bool doEmit) {
    if(asFile) { // file mode
        field_->end(false);
        browseButton_->setIcon(QIcon(":browse"));
        browseButton_->show();
        if(doEmit)
            emit changed(true, field_->text());
    } else { // name mode
        browseButton_->hide();
        if(doEmit) emit changed(false, field_->text());
    }
}

void DbFileWidget::setValue(bool file, QString value) {
    field_->setText(value);
    toggleMode(file, false);
}

void DbFileWidget::textEdited(QString v) {
    emit changed(false, field_->text());
}
