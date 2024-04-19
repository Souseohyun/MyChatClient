#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QWidget>
#include <QPainter>

class BackgroundWidget : public QWidget
{
    Q_OBJECT
public:
    BackgroundWidget(QColor qco,QWidget *parent = nullptr) : color(qco),QWidget(parent) {
        setAttribute(Qt::WA_TranslucentBackground);
    }
    QColor color;
protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.fillRect(rect(), color); // 底色板的颜色和透明度
    }
};

#endif // BACKGROUNDWIDGET_H
