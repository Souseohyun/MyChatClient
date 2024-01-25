#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QFileInfo>

#include <boost/asio.hpp>
//shadow effect
#include <QGraphicsDropShadowEffect>

#include "chatwindow/chatwindow.h"
#include "base/searchbar.h"
#include "chatsession/listwidget.h"
#include "sqlite/sqldatabase.h"
#include "leftmenu/leftbar.h"

class ClientWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit ClientWindow(boost::asio::ip::tcp::socket socket,int& id,QWidget *parent = nullptr);

    // 确保在 ClientWindow 销毁时清理 ChatWindow 实例
    ~ClientWindow() {
        qDeleteAll(chatWindows);
        chatWindows.clear();
    }

protected:
    //拖动窗口
    void mousePressEvent(QMouseEvent *event) override;  //鼠标点击
    void mouseMoveEvent(QMouseEvent *event) override;   //鼠标移动
    void mouseReleaseEvent(QMouseEvent *event) override;//鼠标释放
    bool isPressedWidget;
    QPoint last;

signals:


public slots:
    void OnCellViewSonSelected(int userId,QString name);
    void updateReceivedHeader(const QByteArray& headData);

    //searchbar slots
    void searchSessions();
    void showAllSessions(); //用于点击searchbar中的closeBtn后，重新显示所有会话
    //button slots
    void minimizeWindow();
    void closeWindow();
    void topWindow();
    void maximizeWindow();

    //message recv(chatNet emit signal,client slote action)
    void onMessageReceived(const QString& message, int srcId, int destId);
    //reflush listwidget's cell last message
    void onMessageAdded(int userId, const QString& lastMessage);

    //image server emit signal ,download image in data/heads/id.jpg
    void downloadInHead(int id,const QByteArray& imageData);

private:
    int         myId;
    QPixmap     myHead;

    LeftBar *leftBar;                   //左侧长条菜单

    SearchBar *searchBar;               //搜索栏

    ListWidget* sessionListWidget;      //左侧会话栏

    QPushButton* minimizeButton;
    QPushButton* closeButton;

    QMap<int, ChatWindow*> chatWindows; // 映射 CellViewSon ID 到 ChatWindow 实例

    SqlDataBase* sqlDb;                 //SQLite


    NetworkManager chatNet_;
    NetworkManager imageNet_;

public:

    void testSession();
    void CreateShadow();
    void CreateChatWindow(int userId,QString chatName);
    void CreateButthon();


};

#endif // CLIENTWINDOW_H
