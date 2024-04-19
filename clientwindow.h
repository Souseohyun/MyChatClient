#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QFileInfo>

#include <boost/asio.hpp>
//shadow effect
#include <QGraphicsDropShadowEffect>

#include "backgroundwidget.h"
#include "chatwindow/chatwindow.h"
#include "base/searchbar.h"
#include "chatsession/listwidget.h"
#include "sqlite/sqldatabase.h"
#include "leftmenu/leftbar.h"
#include "leftmenu/contactslistwidget.h"
#include "leftmenu/findfriendwnd.h"
#include "leftmenu/creategroupwnd.h"

#include "settingswnd.h"

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
    void signalSendMessage(const quint8 &, const nlohmann::json &);

public slots:
    void OnCellViewSonSelected(int userId,QString name);
    void updateReceivedHeader(const QByteArray& headData);

    //searchbar slots
    void searchSessions();
    void showAllSessions(); //用于点击searchbar中的closeBtn后，重新显示所有会话

    void sltMenuSelected(QAction *action);

    //button slots
    void minimizeWindow();
    void closeWindow();
    void topWindow();
    void maximizeWindow();

    //message recv(chatNet emit signal,client slote action)
    void onMessageReceived(const QString& html,const QString& text, int srcId, int destId);
    //reflush listwidget's cell last message
    void onMessageAdded(int userId, const QString& lastMessage);

    //image server emit signal ,download image in data/heads/id.jpg
    void downloadInHead(int id,const QByteArray& imageData);

    //show hided contacts/session ListWidget
    void showContactsListWidget();
    void showSessionListWidget();

    void sltOpenDialog(Cell*);
    void contactIsClicked(Cell*);
    //添加一个会话cell到会话栏中
    void sltAddChat(Cell* );
    //查找好友
    void sltFind(const nlohmann::json&);
    //我主动添加别人好友
    void sltAddFriend(const nlohmann::json& js);
    //我被动被请求添加好友
    void sltAddRecv(const nlohmann::json& js);
    //点击请求消息后的弹窗
    void sltOpenAddFriendWnd(Cell *cell);
    //点击同意加好友或反对后，发回服务器信息
    void sltAddResult(const nlohmann::json& js);
    //乙方做出了请求回应
    void sltAddReply(const nlohmann::json& js);

    //群聊窗口建立槽
    void sltCreateGroup(const nlohmann::json& js);

    //服务器回送创建群聊  接收此结果处理的客户端槽函数
    void sltCreateGroupRelpy(const nlohmann::json& js);
private:
    BackgroundWidget *backgroundWidget = nullptr; // 底色板
    BackgroundWidget *backgroundWidget2= nullptr;

    SettingsWnd *settings;

    int         myId;
    QPixmap     myHead;

    LeftBar *leftBar;                   //左侧长条菜单

    SearchBar *searchBar;               //搜索栏

    QMenu *addMenu;

    CreateGroupWnd *newGroup;

    Cell *selectedMenuCell = nullptr;
    Cell *selectedCell = nullptr;

    QStackedWidget* centerWidget;       //左侧管理会话栏和联系人界面的stack


    ListWidget* sessionListWidget;      //左侧会话栏

    ContactsListWidget* contactsListWidget; //默认隐藏的联系人界面

    QPushButton* minimizeButton;
    QPushButton* closeButton;

    QMap<int, ChatWindow*> chatWindows; // 映射 CellViewSon ID 到 ChatWindow 实例

    SqlDataBase* sqlDb;                 //SQLite


    NetworkManager chatNet_;
    NetworkManager imageNet_;


public:
    void SettingShow();

    Cell *isIDExist(int);
    void InsertCelltoSessionList(Cell*);
    void CreateBackgroundWidget();
    void testSession();
    void CreateShadow();
    void CreateChatWindow(int userId,QString chatName);
    void CreateButthon();
    void SetAddMenuStyle();

    void InitSessionList();
    void OnSonMenuSelected(QAction* action);

    void setSelectedMenuCell(Cell*,QMenu*);
};

#endif // CLIENTWINDOW_H
