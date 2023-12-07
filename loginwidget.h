#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include "networkmanager.h"
#include "businesswidget.h"

//debug messagebox
#include <QMessageBox>

//pic paint
#include <QWidget>
#include <QStyle>
#include <QPixmap>
#include <QPainter>

//mouse move
#include<QPoint>
#include<QMouseEvent>

//windows
#include <windows.h>
#include <QGraphicsDropShadowEffect>

//json
#include <nlohmann/json.hpp>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

//cpp
#include <iostream>

#pragma comment(lib, "dwmapi.lib")

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

private slots:
    void on_hideButton_clicked();

    void on_closeButton_clicked();

    void on_pushButton_clicked();

    void onLoginResponseReceived(bool success, const QString& message);
protected:


    void mousePressEvent(QMouseEvent *event) override;  //鼠标点击
    void mouseMoveEvent(QMouseEvent *event) override;   //鼠标移动
    void mouseReleaseEvent(QMouseEvent *event) override;//鼠标释放

private:
    Ui::LoginWidget *ui;
    bool isPressedWidget;
    QPoint last;
    NetworkManager networkManager_;

    void CreatHeadPic();



};

#endif // LOGINWIDGET_H
