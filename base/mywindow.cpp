#include "mywindow.h"
#include <QDebug>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QtMath>
#include <QBitmap>
#include <QDateTime>
#include <QPainter>
#include <QStyleOption>

//背景图动态显示，手工更替
const QString MyWindow::LW_BACKGOUND_PREFIX = ":/login/img_";
const quint8 MyWindow::LW_BACKGOUND_NUM     = 5;


MyWindow::MyWindow(QWidget *parent) : QWidget(parent)
{
    this->setWindowFlag(Qt::FramelessWindowHint);

    m_bkgOpacityInterval = 0.005;
    m_currentBkgIdx = 0;
    m_increasing = true;

    m_bkgTimer = new QTimer(this);
    connect(m_bkgTimer,SIGNAL(timeout()),this,SLOT(changeBackground()));
    m_bkgTimer->start(10);
}


void MyWindow::paintEventForRoundAndBorder(int roundRadius, int borderWidth)
{
    // 绘制圆角窗口
#if 1
    QBitmap maskBmp(this->size());
    maskBmp.fill();
    QPainter maskBmpPainter(&maskBmp);
    maskBmpPainter.setPen(Qt::NoPen);
    maskBmpPainter.setBrush(Qt::black);
    maskBmpPainter.setRenderHint(QPainter::Antialiasing, true); //抗锯齿
    maskBmpPainter.drawRoundedRect(this->rect(), roundRadius, roundRadius);
    setMask(maskBmp);
#endif

    // 绘制边框阴影
    QPainter painter(this);
    QPixmap border(":/win_border.png");
    QRect topRect(0, 0, this->rect().width(), borderWidth);
    QRect topRectSource(5, 0, border.width()-10, 5);
    QRect leftRect(0, 0, borderWidth, this->rect().height());
    QRect leftRectSource(0, 5, 5, border.height()-10);
    QRect rightRect(this->rect().width()-borderWidth, 0, borderWidth, this->rect().height());
    QRect rightRectSource(border.width()-5, 5, 5, border.height()-10);
    QRect bottomRect(0, this->rect().height()-borderWidth, this->rect().width(), borderWidth);
    QRect bottomRectSource(5, border.height()-5, border.width()-10, 5);

    painter.drawPixmap(topRect, border, topRectSource);
    painter.drawPixmap(leftRect, border, leftRectSource);
    painter.drawPixmap(rightRect, border, rightRectSource);
    painter.drawPixmap(bottomRect, border, bottomRectSource);
}


void MyWindow::paintEvent(QPaintEvent *e)
{
    qint8 nextBkgIdx = getNextBkgIdx();
    paintBackground(m_currentBkgIdx, 1);
    paintBackground(nextBkgIdx, m_bkgOpacity);

    paintEventForRoundAndBorder(10,1);

    return QWidget::paintEvent(e);
}


void MyWindow::changeBackground()
{
    m_bkgOpacity = m_bkgOpacity + m_bkgOpacityInterval;
    if(m_bkgOpacity > 1.0){
        m_currentBkgIdx = getNextBkgIdx();
        m_bkgOpacity = 0;
    }

    this->update();
}


qint8 MyWindow::getNextBkgIdx()
{
    qint8 nextBkgIdx = m_currentBkgIdx;
    if(m_increasing == true){
        if(++nextBkgIdx >= LW_BACKGOUND_NUM && m_currentBkgIdx-1 >= 0){
            m_increasing = false;
            nextBkgIdx = m_currentBkgIdx-1;
        }
    }else if(m_increasing == false){
        if(--nextBkgIdx  < 0 && m_currentBkgIdx+1 < LW_BACKGOUND_NUM){
            m_increasing = true;
            nextBkgIdx = m_currentBkgIdx+1;
        }
    }
    return nextBkgIdx;
}


void MyWindow::paintBackground(qint8 bkgIdx, qreal bkgOpacity)
{
    QPainter painter(this);
    QPixmap background;
    background.load(QString("%1%2").arg(LW_BACKGOUND_PREFIX).arg(bkgIdx));
    painter.setOpacity(bkgOpacity);
    painter.drawPixmap(0, 0, background);
}

