/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_DBFILEWIDGET_H_
#define _SEQUELJOE_DBFILEWIDGET_H_

#include <QWidget>

class QLineEdit;
class QToolButton;

class DbFileWidget : public QWidget {
    Q_OBJECT
public:
    explicit DbFileWidget(QWidget* parent = 0);
    virtual QSize sizeHint() const override;
    void setValue(bool file, QString value);

signals:
    void changed(QString);

protected slots:
    void browseButtonClicked();
    void toggleMode(bool file, bool doEmit = true);
    void textEdited(QString);

private:
    QLineEdit* field_;
    QToolButton* browseButton_;
};

#endif // _SEQUELJOE_DBFILEWIDGET_H_
