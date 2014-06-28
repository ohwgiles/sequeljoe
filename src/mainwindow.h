/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_MAINWINDOW_H_
#define _SEQUELJOE_MAINWINDOW_H_

#include <QMainWindow>

class QListWidgetItem;
class TabWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void updateTabName(QWidget*,QString);
    void handleTabChanged(int);
    void newTab();
    void handleTabClosed(int index);
    virtual void closeEvent(QCloseEvent *);

private:
    TabWidget* tabs_;
};

#endif // _SEQUELJOE_MAINWINDOW_H_
