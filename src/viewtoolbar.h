/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_VIEWTOOLBAR_H_
#define _SEQUELJOE_VIEWTOOLBAR_H_

#include <QToolBar>

class QActionGroup;
class QComboBox;

class ViewToolBar : public QToolBar
{
    Q_OBJECT
public:
    enum Panel {
        PANEL_CONTENT,
        PANEL_STRUCTURE,
        PANEL_QUERY
    };

    ViewToolBar(QWidget* parent = 0);
    void setCurrentDatabase(QString);

public slots:
    void enableAll(bool enabled = true);
    void populateDatabases(QStringList names);

signals:
    void panelChanged(ViewToolBar::Panel);
    void disconnect();
    void dbChanged(QString);

private slots:
    void showContent() { emit panelChanged(PANEL_CONTENT); }
    void showStructure() { emit panelChanged(PANEL_STRUCTURE); }
    void showQuery() { emit panelChanged(PANEL_QUERY); }
    void dbComboModified(int);

private:
    QAction* addAction(QString icon, QString label, const char* slot);
    QActionGroup* group_;
    QComboBox* dbSelect_;
};

#endif // _SEQUELJOE_VIEWTOOLBAR_H_
