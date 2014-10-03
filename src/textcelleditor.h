/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_TEXTCELLEDITOR_H_
#define _SEQUELJOE_TEXTCELLEDITOR_H_

#include <QDialog>

class QPlainTextEdit;

class TextCellEditor : public QDialog
{
    Q_OBJECT
public:
    explicit TextCellEditor(QWidget *parent = 0);
    QSize sizeHint() const override { return QSize(300,300); }

    void setContent(const QString& txt);
    QString content() const;

private:
    QPlainTextEdit* editor;
};

#endif // _SEQUELJOE_TEXTCELLEDITOR_H_
