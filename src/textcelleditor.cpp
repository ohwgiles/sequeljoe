#include "textcelleditor.h"

#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QDebug>
TextCellEditor::TextCellEditor(QWidget *parent) :
    QDialog(parent)
{
    this->setModal(true);
    qDebug() << parent;
    QBoxLayout* l = new QVBoxLayout(this);
    editor_ = new QPlainTextEdit(this);
    setWindowTitle("Edit Text");
//setGeometry(parent->geometry());
    l->addWidget(editor_);
    //setLayout(l);
}
void TextCellEditor::setContent(const QString& txt) {
    editor_->document()->setPlainText(txt);
}

QString TextCellEditor::content() const {
    return editor_->document()->toPlainText();
}
