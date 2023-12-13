#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <nlohmann/json.hpp>


#include "bubbleinfo.h"
#include "bubbleitemwidget.h"
#include "QNChatMessage.h"
#include "../networkmanager.h"

namespace Ui {
class ChatWindow;
}


class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(boost::asio::ip::tcp::socket socket,int& user_id,
                        QWidget *parent = nullptr);
    ~ChatWindow();

private slots:
    void on_toolButton_clicked();


public slots:
    void displayReceivedData(const QString& data);

public:
    void onTextEidtReturnPressed();

protected:
    void mousePressEvent(QMouseEvent *event) override;  //鼠标点击
    void mouseMoveEvent(QMouseEvent *event) override;   //鼠标移动
    void mouseReleaseEvent(QMouseEvent *event) override;//鼠标释放
    void resizeEvent(QResizeEvent* event) override;
    //事件过滤器
    bool eventFilter(QObject *obj, QEvent *event) override;

    bool isPressedWidget;
    QPoint last;


//组件信息
private:
    QPixmap      myPic;
    QListWidget* listWidget;
    QLineEdit* lineEdit;
    QPushButton* sendButton;


    void addMessage(const QString &text, const QString &time, QNChatMessage::User_Type userType);


private:
    Ui::ChatWindow *ui;
    NetworkManager chatNetworkManager_;


//用户信息
private:
    int userId_;
};

#endif // CHATWINDOW_H
