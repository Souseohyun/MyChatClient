#include "roundlabel.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QPainterPath>
RoundLabel::RoundLabel(QWidget *parent,QString iconPath)
    : QLabel(parent)
{
    // pixmap = QPixmap(iconPath);
    // pixmap.scaled(this->width(),this->height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    if (!iconPath.isEmpty()) {
        QPixmap originalPixmap(iconPath);
        // Make sure the original pixmap is not null
        if (!originalPixmap.isNull()) {
            // Scale the pixmap to fit the RoundLabel's current size while maintaining the aspect ratio
            pixmap = originalPixmap.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
    }else{
        qDebug()<<"iconPath is Empty ERROR";
    }
}

void RoundLabel::setPixmap(QString iconpath)
{
    // pixmap = QPixmap(iconpath);
    // pixmap.scaled(this->width(),this->height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    // update();

    pixmap = QPixmap(iconpath);
    // Make sure the pixmap is not null
    if (!pixmap.isNull()) {
        // Scale the pixmap to fit the RoundLabel's current size while maintaining the aspect ratio
        pixmap = pixmap.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        update(); // Schedule a repaint event
    }

}

void RoundLabel::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform,true);// 抗锯齿

    QPainterPath path;
    int radius = this->width()/2;
    path.addEllipse(0,0,radius * 2,radius*2);
    painter.setClipPath(path);

    painter.drawPixmap(QRect(0,0,radius*2,radius*2),pixmap);

    return QLabel::paintEvent(e);
}
