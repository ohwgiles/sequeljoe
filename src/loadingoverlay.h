/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_LOADINGOVERLAY_H_
#define _SEQUELJOE_LOADINGOVERLAY_H_

#include <QWidget>

class Spinner;
class QTimeLine;

class LoadingOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit LoadingOverlay(QWidget *parent = 0);
    virtual ~LoadingOverlay();

protected:
    virtual void paintEvent(QPaintEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

private:
    static Spinner* spinner;
    QTimeLine* timeline_;
};

#endif // _SEQUELJOE_LOADINGOVERLAY_H_