#ifndef REGISTERWND_H
#define REGISTERWND_H

#include "base/mywindow.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMouseEvent>

#include <nlohmann/json.hpp>

class RegisterWndPrivate : public MyWindow
{
    Q_OBJECT

public:
    RegisterWndPrivate(QWidget *parent = nullptr);

signals:
    void closeWindow();
    void signalRegister(const nlohmann::json &json);

private:
    QLabel* m_notifyMsg;

    QPushButton* m_menuCloseBtn;         // 菜单栏关闭按钮
    QPushButton* m_menuMinBtn;           // 菜单栏最小化按钮

    QLabel *welcome;
    QLabel *logo;

    QLabel *nicknameLabel;
    QLabel *usernameLabel;
    QLabel *passwordLabel;

    QLineEdit *nicknameEdit;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;

    QString username;
    QString password;

    QPushButton *okBtn;
    QPushButton *cancelBtn;
    QLabel *idLabel;


public slots:
    void sltBtnClicked();
    void closeWnd();
    void sltRegisterReply(const nlohmann::json&);
};


class RegisterWnd : public QWidget
{
    Q_OBJECT

public:
    RegisterWnd(QWidget *parent = nullptr);

signals:
    void closeWindow(QPoint);
    void signalRegister(const nlohmann::json &json);

public slots:
    void sltCloseWnd();
    void sltRegisterOK(const nlohmann::json&);

private:
    RegisterWndPrivate *mainWnd;

    QPointF startPos;
    QPointF pressedPoint; // for moving window
    bool   isPressed = false;

protected:
    // 重写鼠标操作以实现移动窗口
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // REGISTERWND_H
