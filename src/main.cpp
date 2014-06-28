/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include <QApplication>
#include "mainwindow.h"
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

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationDomain("sequeljoe.org");

#ifdef __APPLE__
    QApplication::setStyle("cocoa");
#endif
    QApplication a(argc, argv);
    // todo: get this from the environment/compile flags
    //a.addLibraryPath("/Users/olivergiles/Qt/5.1.1/clang_64/plugins/sqldrivers");

#ifdef __APPLE__
    // small hack to fix the weirdly large font size
    a.setStyle(new MacFontStyle);
#endif

    MainWindow w;
    w.show();    

    return a.exec();
}

