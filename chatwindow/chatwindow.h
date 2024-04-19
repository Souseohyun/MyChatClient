#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QFileDialog>
#include <QTextDocument>

#include <nlohmann/json.hpp>
#include <QMenu>

#include "bubbleinfo.h"
#include "bubbleitemwidget.h"
#include "QNChatMessage.h"
#include "../networkmanager.h"
#include "clientapi/screenwidget.h"

#include "base/mybutton.h"

namespace Ui {
class ChatWindow;
}

//它的存在单纯就是为了被动的在分离的线程中一次性加载气泡到list里
//函数：startAsyncLoadMsg
struct MessageData {
    QString html;
    QString text;
    QString timeString;
    QNChatMessage::User_Type userType;
    int srcId;
};

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

    //发送图片按钮点击
    void onPictureButtonClicked();
    //截图按钮
    void onScreenshotClicked();
    //截图类传来的信号
    void handleScreenshot(const QPixmap &screenshot);
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
    void addMessage(const QString &html, const QString &text, const QString &time, QNChatMessage::User_Type userType,int addUserId);
    //void addMessage(const QString text, const QString time, QNChatMessage::User_Type userType,int addUserId);

    void addX(const QString &html,const QString &text,const QString &time, QNChatMessage::User_Type userType,int addUserId,bool isHistoryMessage = false,bool isLastHistory = false);

    QSize  calculateDocumentSize(const QString& html);

    void setWindowName(const QString& name);
    void setUIStyle();

    //消息存入本地数据库，云端数据库经过服务器时先转发再顺带存了
    void SaveMsgToDB(BubbleInfo* bf);

    //从数据库中读取消息  废弃，新版本startAsyncLoadMsg
    //void LoadAllMsgFromDB();
    //剥离完整html内容为仅p标签内内容
    QString extractContentFromP(const QString &html);
    //给message套上完整的html内容待显示
    QString RecreateHtml(const QString &ptick);
    //给message去掉标签
    QString RemoveHtmlTags(const QString &input);
    //异步启动加载历史信息操作
    void startAsyncLoadMsg();
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
