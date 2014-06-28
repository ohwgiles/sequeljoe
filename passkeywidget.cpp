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
#include <QPushButton>

#include <QFileDialog>

PassKeyWidget::PassKeyWidget(QWidget *parent) : QWidget(parent)
{
    //QBoxLayout* layout = new QHBoxLayout(this);
    //layout->setSpacing(0);
    field_ = new QLineEdit(this);
//    QMessageBox* box = new QMessageBox(QMessageBox::Information, "title", "text");
    keyButton_ = new QPushButton(this);

    keyButton_->setStyleSheet("QPushButton{border:none;}");
    //field_->setStyleSheet("QLineEdit{padding-right:20px}");
//    connect(keyButton_, SIGNAL(clicked()), box, SLOT(show()));
    //layout->setContentsMargins(0,0,0,0);
    //layout->addWidget(field_);
    //layout->addWidget(b);
    //field_->setTextMargins(0,0,keyButton_->iconSize().width(),0);
    QSize hint = field_->sizeHint();
    int sz = hint.height() * 0.7;
    keyButton_->setIconSize(QSize(sz,sz));
    int margin = (hint.height() - sz) / 2;
    keyButton_->setGeometry(
                hint.width() - sz - margin,
                margin,
                sz,
                sz
                );
//field_->setFixedWidth(field_->width()-30);//50,30);
//    QMargins m = field_->textMargins();
//    m.setRight(m.right()+sz+margin);
    //field_->setTextMargins(m);
    field_->setTextMargins(0,0,sz,0);
    field_->setFixedWidth(hint.width());
    connect(keyButton_, SIGNAL(clicked()), this, SLOT(keyButtonClicked()));

    connect(field_, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)));
    toggleMode(false);
}

QSize PassKeyWidget::sizeHint() const
{//return field_->sizeHint() - field_->textMargins().right();
    QSize s = field_->sizeHint();
    s.setWidth(s.width()-field_->textMargins().right());
    return s;
}
/*
QSize PassKeyWidget::minimumSizeHint() const
{return field_->minimumSizeHint();
    QSize s = field_->minimumSizeHint();
    s.setWidth(s.width()-20);
    return s;

}
*/
//bool PassKeyWidget::hasKey() const {
//    return field_->isReadOnly();
//}

void PassKeyWidget::keyButtonClicked()
{
    bool isKeyMode = (field_->isReadOnly());
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

void PassKeyWidget::toggleMode(bool key, bool doEmit)
{
    if(key) { // key mode
        field_->setEchoMode(QLineEdit::Normal);
        field_->setReadOnly(true);
        field_->end(false);
        //field_->scroll();
        keyButton_->setIcon(QIcon(":remove"));
        if(doEmit) emit changed(true, field_->text());

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

void PassKeyWidget::textEdited(QString v)
{
    emit changed(false, field_->text());
}
