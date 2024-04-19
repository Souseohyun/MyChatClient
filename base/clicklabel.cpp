#include "clicklabel.h"



ClickLabel::ClickLabel(QWidget *parent) : QLabel(parent)
{

}

void ClickLabel::mousePressEvent(QMouseEvent *)
{
    emit clickSignal();
}

void ClickLabel::enterEvent(QEvent *)
{
    this->setStyleSheet("font-size: 12px;font-family:Microsoft YaHei; color: #12b7f5");
    setCursor(Qt::PointingHandCursor);
}

void ClickLabel::leaveEvent(QEvent *)
{
    this->setStyleSheet("font-size: 12px;font-family:Microsoft YaHei; color: #a6a6a6");
    setCursor(Qt::ArrowCursor);
}
