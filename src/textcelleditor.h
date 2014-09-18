#ifndef TEXTCELLEDITOR_H
#define TEXTCELLEDITOR_H

#include <QDialog>

class QPlainTextEdit;

class TextCellEditor : public QDialog
{
    Q_OBJECT
public:
    explicit TextCellEditor(QWidget *parent = 0);
    void setContent(const QString& txt);
    QString content() const;
QSize sizeHint() const { return QSize(300,300); }
signals:

public slots:

private:
    QPlainTextEdit* editor_;
};

#endif // TEXTCELLEDITOR_H
