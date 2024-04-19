#ifndef DATABASE_H
#define DATABASE_H

#include "chatwindow/bubbleinfo.h"

#include "myconfig.h"


#include <nlohmann/json.hpp>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <QMutex>


class Cell;

class SqlDataBase : public QObject
{
    Q_OBJECT
public:
    explicit SqlDataBase(QObject *parent = nullptr);
    ~SqlDataBase();

    //待废弃 log 2024.1.7
    bool openDb(const QString &dataName);

    //新版本重载
    bool openDb(const int &userId);
    // 单实例运行
    static SqlDataBase *Instance()
    {
        static QMutex mutex;
        if (self == nullptr) {
            QMutexLocker locker(&mutex);

            if (!self) {
                self = new SqlDataBase();
            }
        }
        return self;
    }

    // 添加历史聊天记录
    void addHistoryMsg(BubbleInfo *info);
    // 添加好友
    //void addFriend(const int &myID, const int &friendID);
    void addFriend(const int &myID, const int &friendID,
                   const QString &name, const QString &teamname,
                   const QString &markname);
    // 添加群组
    void addGroup(const int &myID, const int &groupID);

    // 创建群
    void createGroup(int groupId, const QString &groupName, int adminId, const QString &headImage);

    // 删除好友
    bool deleteMyFriend(const int &myID, const int &friendID);

    // 获取我的好友
    nlohmann::json getMyFriends() const;
    // 获取我的群组
    nlohmann::json getMyGroups() const;
    // 获取我的分组
    nlohmann::json getMySubgroup() const;
    // 获取我的聊天列表
    QJsonArray getMyChatList() const;
    // 获取好友信息
    nlohmann::json getFriendInfo(int id) const;
    // 获取群信息
    nlohmann::json getGroupInfo(int id) const;

    // 判断改好友是否已经是我的好友了
    bool isMyFriend(const int &friendID);
    // 判断是否已经加入该群组了
    bool isInGroup(const int &groupID);
    // 判断是否是一个群的群主
    bool isAdmin(int id,int groupID);

    void updateFileMsg(BubbleInfo *info);

    // 获取历史聊天记录,id表示对方的id,tag标记这是私聊还是群聊窗口,count表示需加载的记录数（-1代表all）
    QVector<BubbleInfo *> QueryMsgHistory(int id,int tag, int count);

    // 测试使用，打印数据库中的所有信息
    void queryAll();



    QString GetHeadById(int friendId);
    QList<Cell*> getAllChatSessions();

    std::vector<int> getAllFriendIds();
signals:

public slots:

private:
    static SqlDataBase *self;

    // 数据库管理
    QSqlDatabase userdb;
};


#endif // DATABASE_H
