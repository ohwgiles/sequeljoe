/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_PASSKEYWIDGET_H_
#define _SEQUELJOE_PASSKEYWIDGET_H_

#include <QWidget>
class QLineEdit;
class QPushButton;
class PassKeyWidget : public QWidget
{
    Q_OBJECT
public:
    PassKeyWidget(QWidget* parent = 0);
    QSize sizeHint() const;
    //QSize minimumSizeHint() const;

    void setValue(bool key, QString value);
    //bool hasKey() const;
//    void setModePassword();
//    void setModeKey();
private:
    QLineEdit* field_;
    QPushButton* keyButton_;
signals:
    void changed(bool, QString);
protected slots:
    void keyButtonClicked();
    void toggleMode(bool key, bool doEmit = true);
    void textEdited(QString);
};

#endif // _SEQUELJOE_PASSKEYWIDGET_H_
