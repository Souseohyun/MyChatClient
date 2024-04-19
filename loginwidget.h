#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include "networkmanager.h"

//base
#include "base/clicklabel.h"

//
#include "registerwnd.h"

//debug messagebox
#include <QMessageBox>

//pic paint
#include <QWidget>
#include <QStyle>
#include <QPixmap>
#include <QPainter>
#include <QMovie>

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
#include <string>

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

    void onLoginResponseReceived(bool success, const QString& message,int user_id);
    //携带好友信息的信号槽
    void onLoginResponseReceivedWithFriends
        (bool success, const QString& message,
         int user_id, const nlohmann::json& friends);

    //携带未读信息的信号槽
    void onMsgResponseReceived(const int user_id,const nlohmann::json& msgJson);

    //展开注册界面
    void openRegisterWnd();
    //注册界面关闭，show自己
    void sltRegisterClose(QPoint);
    //将注册界面携带的信号参数发给服务器
    void sltRegisterGo(const nlohmann::json& js);

protected:


    void mousePressEvent(QMouseEvent *event) override;  //鼠标点击
    void mouseMoveEvent(QMouseEvent *event) override;   //鼠标移动
    void mouseReleaseEvent(QMouseEvent *event) override;//鼠标释放

private:
    Ui::LoginWidget *ui;
    bool isPressedWidget;
    QPoint last;
    NetworkManager networkManager_;

    bool haveDB;

    void CreatHeadPic();
    void CreatBackgroud();
    void CreatLogo();
    void CreatShadow();

    QLabel *MsgNotify;      //信息反馈界面
    void ShowNotify(QString msg);
    void HideNotify();

    ClickLabel *registerAccount;    //注册
    RegisterWnd *registerWnd;       //注册的界面
};

#endif // LOGINWIDGET_H
