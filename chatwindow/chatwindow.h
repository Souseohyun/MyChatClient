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
#include <QMenu>

#include "bubbleinfo.h"
#include "bubbleitemwidget.h"
#include "QNChatMessage.h"
#include "../networkmanager.h"

#include "base/mybutton.h"

namespace Ui {
class ChatWindow;
}


class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(boost::asio::ip::tcp::socket socket,int& user_id,
                        QWidget *parent = nullptr);
    explicit ChatWindow(NetworkManager& chatNet,
                        NetworkManager& imageNet,
                        int& user_id,
                        int& son_id,
                        QString name,
                        QPixmap myHead,
                        QPixmap youHead,
                        QWidget *parent = nullptr);
    ~ChatWindow();
signals:
    void messageAdded(int userId, const QString& lastMessage);
private slots:
    void on_toolButton_clicked();


public slots:

    void updateReceivedHeader(const QByteArray& headData);
    void updateReceivedImage(const QByteArray& imageData);
public:
    void onTextEidtReturnPressed();

protected:
    /*
    void mousePressEvent(QMouseEvent *event) override;  //鼠标点击
    void mouseMoveEvent(QMouseEvent *event) override;   //鼠标移动
    void mouseReleaseEvent(QMouseEvent *event) override;//鼠标释放
    */
    void resizeEvent(QResizeEvent* event) override;
    //事件过滤器
    bool eventFilter(QObject *obj, QEvent *event) override;

    bool isPressedWidget;
    QPoint last;


//组件信息
private:
    QPixmap      myPic;
    QPixmap      youPic;
    QLabel*      chatName;
    QListWidget* listWidget;
    QLineEdit* lineEdit;
    QPushButton* sendButton;

    MyButton    *emoji;
    MyButton    *picture;
    MyButton    *file;
    MyButton    *screenshot;

    QMenu       *emojiMenu;
public:
    void addMessage(const QString &text, const QString &time, QNChatMessage::User_Type userType,int addUserId);
    //void addMessage(const QString text, const QString time, QNChatMessage::User_Type userType,int addUserId);

    void addX();

    QSize  calculateDocumentSize(const QString& html);

    void setUIStyle();

private:
    Ui::ChatWindow *ui;
    NetworkManager& chatNetworkManager_;
    NetworkManager& imageNetworkManager_;


//用户信息
private:
    int userId_;
    int toId_;
};

#endif // CHATWINDOW_H
