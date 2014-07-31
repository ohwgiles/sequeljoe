/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "mainwindow.h"
#include "notify.h"

#include <QApplication>
#include <QProxyStyle>
#include <QMenu>

class MacFontStyle : public QProxyStyle {
protected:
    void polish(QWidget *w) {
        QMenu* mn = dynamic_cast<QMenu*>(w);
        if(!mn && !w->testAttribute(Qt::WA_MacNormalSize))
            w->setAttribute(Qt::WA_MacSmallSize);
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationDomain("sequeljoe.org");
    notify = new Notifier();

    QApplication a(argc, argv);

#ifdef __APPLE__
    // prevents the font size from appearing overly large on OSX
    a.setStyle(new MacFontStyle);
#endif

    MainWindow w;
    w.show();    

    return a.exec();
}

