/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_TABWIDGET_H_
#define _SEQUELJOE_TABWIDGET_H_

#include <QTabWidget>

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0);
    // override to insert before end
    int addTab(QWidget *widget, const QString & label) {
        return insertTab(0, widget, label);
    }
    int lastActiveIndex() const { return lastIndex_; }
private:
    QWidget* empty_;
    int lastIndex_;
signals:
    void newTab();
private slots:
    void plusToEnd();
    void checkNewTab(int);
};

#endif // _SEQUELJOE_TABWIDGET_H_
