/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "textcelleditor.h"

#include <QVBoxLayout>
#include <QPlainTextEdit>

TextCellEditor::TextCellEditor(QWidget *parent) :
    QDialog(parent)
{
    setModal(true);
    QBoxLayout* l = new QVBoxLayout(this);
    editor = new QPlainTextEdit(this);
    setWindowTitle("Edit Text");
    l->addWidget(editor);
}

void TextCellEditor::setContent(const QString& txt) {
    editor->document()->setPlainText(txt);
}

QString TextCellEditor::content() const {
    return editor->document()->toPlainText();
}
