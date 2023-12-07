// BubbleItemWidget.h
#ifndef BUBBLEITEMWIDGET_H
#define BUBBLEITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QPainter>
#include <QStyleOption>
#include <QRegion>
#include <QPainterPath>


#include "BubbleInfo.h"

class BubbleItemWidget : public QWidget {
    Q_OBJECT
public:
    explicit BubbleItemWidget(QWidget *parent = nullptr);
    void setInfo(const BubbleInfo &info); // Method to set the data on the widget
    QSize calculateSize();
    QSize sizeHint() const override;
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *messageLabel;   //文本
    QLabel *timestampLabel; //时间戳
    QLabel *avatarLabel;    //头像
    BubbleInfo bubbleInfo;  //气泡相关属性
};

#endif // BUBBLEITEMWIDGET_H
