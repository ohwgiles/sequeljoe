/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "loadingoverlay.h"

#include <QPainter>
#include <QPaintEvent>
#include <QApplication>
#include <QTimeLine>

Spinner* LoadingOverlay::spinner = nullptr;

class Spinner {
public:
    Spinner(int rOuter, int rInner) :
        pixmap_(2*rOuter, 2*rOuter)
    {
        QPainterPath path;
        path.setFillRule(Qt::OddEvenFill);
        path.addEllipse(QPointF(rOuter, rOuter), rOuter, rOuter);
        path.addEllipse(QPointF(rOuter, rOuter), rInner, rInner);

        pixmap_.fill(Qt::transparent);
        QPainter p(&pixmap_);

        p.setPen(Qt::NoPen);
        p.setBrush(Qt::transparent);
        p.setRenderHint(QPainter::Antialiasing);
        p.drawPath(path);

        QConicalGradient gradient(QPointF(rOuter,rOuter), 0.0);
        gradient.setColorAt(0.0, Qt::transparent);
        gradient.setColorAt(0.05, QApplication::palette().color(QPalette::Highlight));
        gradient.setColorAt(0.75, Qt::transparent);
        p.setBrush(gradient);
        p.drawPath(path);
        p.end();
    }

    const QPixmap& pixmap() const { return pixmap_; }

private:
    QPixmap pixmap_;
};



LoadingOverlay::LoadingOverlay(QWidget *parent) :
    QWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    if(spinner == nullptr)
        spinner = new Spinner(60, 50);

    timeline_ = new QTimeLine(1000);
    timeline_->setFrameRange(0,360);
    timeline_->setCurveShape(QTimeLine::LinearCurve);
    connect(timeline_, SIGNAL(frameChanged(int)), this, SLOT(repaint()));
    timeline_->setLoopCount(0); // loop forever
}

LoadingOverlay::~LoadingOverlay() {
    delete timeline_;
}

void LoadingOverlay::showEvent(QShowEvent *e) {
    QWidget::showEvent(e);
    timeline_->start();
}

void LoadingOverlay::hideEvent(QHideEvent * e) {
    QWidget::hideEvent(e);
    timeline_->stop();
}

void LoadingOverlay::paintEvent(QPaintEvent * e) {
    QWidget::paintEvent(e);
    QPainter painter(this);
    QRect r = e->rect();

    painter.fillRect(r, QColor(0,0,0,128));

    const QPixmap& px = spinner->pixmap();
    painter.translate(r.width()/2,r.height()/2);
    painter.rotate(timeline_->currentFrame());
    painter.drawPixmap(-px.width()/2,-px.width()/2,spinner->pixmap());
}

