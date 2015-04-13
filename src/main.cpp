/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "mainwindow.h"
#include "notify.h"
#include "dbconnection.h"
#include "tabledata.h"
#include "foreignkey.h"

#include <QApplication>
#include <QProxyStyle>
#include <QMenu>
#include <QSqlQuery>

#ifdef __APPLE__
class MacFontStyle : public QProxyStyle {
protected:
    void polish(QWidget *w) {
        QMenu* mn = dynamic_cast<QMenu*>(w);
        if(!mn && !w->testAttribute(Qt::WA_MacNormalSize))
            w->setAttribute(Qt::WA_MacSmallSize);
    }
};
#endif // __APPLE__

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationDomain("sequeljoe.org");

    QApplication a(argc, argv);

    qRegisterMetaType<ForeignKey>("ForeignKey");
    qRegisterMetaType<QSqlQuery*>("QSqlQuery*");
    qRegisterMetaType<const char*>("const char*");
    qRegisterMetaType<TableMetadata>("TableMetadata");
    qRegisterMetaType<Schema*>("Schema*");

#ifdef __APPLE__
    // prevents the font size from appearing overly large on OSX
    a.setStyle(new MacFontStyle);
#endif // __APPLE__

    MainWindow w;
    w.show();    
    a.exec();
    Notifier::cleanup();
    return 0;
}

