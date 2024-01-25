#include "sqldatabase.h"

#include "chatsession/cell.h"


#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QFile>
#include <QSqlError>




#define DATE_TME_FORMAT     QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss")

SqlDataBase *SqlDataBase::self = nullptr;

SqlDataBase::SqlDataBase(QObject *parent) : QObject(parent)
{

}


SqlDataBase::~SqlDataBase()
{
    if (userdb.isOpen()) {
        userdb.close();
    }
}

bool SqlDataBase::openDb(const QString &dataName)
{

    userdb = QSqlDatabase::addDatabase("QSQLITE");
    userdb.setDatabaseName(dataName);
    if (!userdb.open()) {
        qDebug() << "Open database failed!";
        return false;
    }


    // 创建添加数据表
    QSqlQuery query;

    // 创建我的好友表 id为好友id,group为该好友所在的分组,
    // tag记录关闭程序时聊天列表是否已经有与该用户的对话框，目的是下次打开时初始化聊天列表用的,以下相同
    query.exec("CREATE TABLE MyFriend (id INT PRIMARY KEY, name varchar(50), head varchar(20), subgroup varchar(20), "
               "tag bit ,lastMsg varchar(50), lastTime varchar(20) )");

    // 创建群组表 id为群组id,
    query.exec("CREATE TABLE MyGroup (id INT PRIMARY KEY, name varchar(50), admin INT, head varchar(20), "
               "tag bit ,lastMsg varchar(50), lastTime varchar(20) )");

    query.exec("CREATE TABLE MySubgroup (name varchar(20) PRIMARY KEY, datetime varchar(20) )");

    query.exec(QString("insert into MySubgroup values('默认分组','") + DATE_TME_FORMAT + "')");
    query.exec(QString("insert into MySubgroup values('黑名单','") + DATE_TME_FORMAT + "')");

    // 创建历史聊天表 tag = 0表示私聊消息,tag = 1表示群聊消息,
    // sender = 0表示是我发的消息,sender = 1表示是对方发的消息,sender = 2表示是系统发的消息
    query.exec("CREATE TABLE MsgHistory (id INT PRIMARY KEY, sender INT, "
               "myID INT, yourID INT, groupID INT, tag bit, "
               "type INT, content varchar(500), filesize varchar(30), download bit, "
               "datetime BIGINT)");

    return true;
}

bool SqlDataBase::openDb(const int &userId)
{
    QString strUserId = QString::number(userId);
    QString dataName = MyConfig::strDatabasePath + "user_" + strUserId + ".db";
    QString connectionName = "userConnection_" + strUserId;  // 根据 userId 创建连接名

    if (QSqlDatabase::contains(connectionName)) {
        userdb = QSqlDatabase::database(connectionName); // 获取已存在的数据库连接
    } else {
        userdb = QSqlDatabase::addDatabase("QSQLITE", connectionName); // 创建新的数据库连接
        userdb.setDatabaseName(dataName);
    }

    if (!userdb.open()) {
        qDebug() << "Open database failed for user" << userId;
        return false;
    }

    // 创建添加数据表
    QSqlQuery query(userdb);

    // 创建我的好友表 id为好友id,group为该好友所在的分组,
    // tag记录关闭程序时聊天列表是否已经有与该用户的对话框，目的是下次打开时初始化聊天列表用的,以下相同
    query.exec("CREATE TABLE MyFriend (id INT PRIMARY KEY, name varchar(50), head varchar(20), subgroup varchar(20), "
               "tag bit ,lastMsg varchar(50), lastTime varchar(20) )");

    // 创建群组表 id为群组id,
    query.exec("CREATE TABLE MyGroup (id INT PRIMARY KEY, name varchar(50), admin INT, head varchar(20), "
               "tag bit ,lastMsg varchar(50), lastTime varchar(20) )");

    query.exec("CREATE TABLE MySubgroup (name varchar(20) PRIMARY KEY, datetime varchar(20) )");

    query.exec(QString("insert into MySubgroup values('默认分组','") + DATE_TME_FORMAT + "')");
    query.exec(QString("insert into MySubgroup values('黑名单','") + DATE_TME_FORMAT + "')");

    // 创建历史聊天表 tag = 0表示私聊消息,tag = 1表示群聊消息,
    // sender = 0表示是我发的消息,sender = 1表示是对方发的消息,sender = 2表示是系统发的消息
    query.exec("CREATE TABLE MsgHistory (id INT PRIMARY KEY, sender INT, "
               "myID INT, yourID INT, groupID INT, tag bit, "
               "type INT, content varchar(500), filesize varchar(30), download bit, "
               "datetime BIGINT)");

    return true;
}


void SqlDataBase::addFriend(const int &myID, const int &friendID,
                            const QString &name, const QString &teamname,
                            const QString &markname) {
    // 检查是否已经是好友
    QString strQuery = QString("SELECT * FROM MyFriend WHERE id = %1").arg(friendID);
    QSqlQuery query(userdb);
    query.exec(strQuery);

    if (query.next()) {
        // 查询到有该用户，不添加
        qDebug() << "已经是好友了，无法再次添加: " << friendID;
        return;
    }

    // 插入新的好友记录
    query.prepare("INSERT INTO MyFriend (id, name, head, subgroup, tag, lastMsg, lastTime) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?);");
    query.bindValue(0, friendID);
    query.bindValue(1, /*name*/markname);//权益之举，用markname顶一下name

    //此处头像直接拿本地的
    QString headImage = QString::number(friendID)+".jpg";
    query.bindValue(2, headImage);

    query.bindValue(3, teamname); // 分组信息
    query.bindValue(4, false); // tag 初始值为 false
    query.bindValue(5, ""); // lastMsg 初始值为空
    query.bindValue(6, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")); // 当前时间为最后消息时间

    if (!query.exec()) {
        qDebug() << "添加好友失败: " << query.lastError().text();
    }
}

/*待废弃
void SqlDataBase::addFriend(const int &myID, const int &friendID)
{
    QString strQuery = "SELECT * FROM MyFriend ";
    strQuery.append("WHERE id=");
    strQuery.append(QString::number(friendID));

    QSqlQuery query(strQuery);
    if (query.next()) {
        // 查询到有该用户，不添加
        qDebug() << "已经是好友了，无法再次添加" << query.value(0).toString();
            return;
    }

    // 根据新ID重新创建用户
    query.prepare("INSERT INTO MyFriend (id, name, head, group) "
                  "VALUES (?, ?, ?, ?);");
    query.bindValue(0, friendID);
    query.bindValue(1, "");
    query.bindValue(2, "");
    query.bindValue(3, "我的好友");

    query.exec();

    //此处再向服务器添加一条好友信息
}
*/




void SqlDataBase::addGroup(const int &myID, const int &groupID)
{
    QString strQuery = "SELECT * FROM MyGroup ";
    strQuery.append("WHERE id=");
    strQuery.append(QString::number(groupID));

    QSqlQuery query(strQuery);
    if (query.next()) {
        qDebug() << "已经加入该群了，无法再次加入" << query.value(0).toString();
            return;
    }

    // 根据新ID重新创建用户
    query.prepare("INSERT INTO MYGROUP (id, name, head) "
                  "VALUES (?, ?, ?);");
    query.bindValue(0, groupID);
    query.bindValue(1, "");
    query.bindValue(2, "");

    query.exec();

    //此处再向服务器添加一条加群信息
}

bool SqlDataBase::deleteMyFriend(const int &myID, const int &friendID)
{
    QString strQuery = "SELECT * FROM MyFriend ";
    strQuery.append("WHERE id=");
    strQuery.append(QString::number(friendID));


    QSqlQuery query(strQuery);
    // 删除
    if (query.next()) {
        strQuery = "DELETE FROM MyFriend WHERE id=";
        strQuery.append(QString::number(friendID));

        query = QSqlQuery(strQuery);
        return query.exec();
    }

    // 没有查询到有该用户
    // 向服务器更新消息
    return false;
}

nlohmann::json SqlDataBase::getMyFriends() const
{
    using json = nlohmann::json;

    json myFriends = json::array();

    QString strQuery = "SELECT * FROM MyFriend ";
    QSqlQuery query(strQuery);

    while (query.next()){
        json jsonFriend;
        jsonFriend["id"] = query.value("id").toInt();
        jsonFriend["head"] = query.value("head").toString().toStdString(); // 转换为 std::string
        jsonFriend["name"] = query.value("name").toString().toStdString();
        jsonFriend["lastMsg"]  = query.value("lastMsg").toString().toStdString();
        jsonFriend["subgroup"] = query.value("subgroup").toString().toStdString();
        myFriends.push_back(jsonFriend);
    }

    return myFriends;
}

//根据id获取数据库中头像路径
QString SqlDataBase::GetHeadById(int friendId) {
    QSqlQuery query(userdb);
    QString queryString = QString("SELECT head FROM MyFriend WHERE id = %1").arg(friendId);
    if (query.exec(queryString) && query.next()) {
        return query.value(0).toString();
    } else {
        return QString(); // 返回空字符串表示未找到
    }
}

//用于初始化会话栏
QList<Cell *> SqlDataBase::getAllChatSessions()
{
    QList<Cell*> sessions;
    QSqlQuery query(userdb);

    if (!query.exec("SELECT id, name, head, lastMsg, lastTime FROM MyFriend")) {
        qDebug() << "Error in getChatSessions: " << query.lastError().text();
        return sessions; // 返回空列表
    }

    while (query.next()) {
        Cell* cell = new Cell();
        cell->id = query.value(0).toInt();
        cell->name = query.value(1).toString();
        cell->iconPath = MyConfig::strHeadPath + query.value(2).toString();
        cell->msg = query.value(3).toString();
        cell->type = Cell_FriendChat;
        sessions.append(cell);
    }

    return sessions;

}

//获取该用户对应的数据库中表MyFriend中所有的id，存放至vector中
std::vector<int> SqlDataBase::getAllFriendIds()
{
    std::vector<int> friendIds;
    QSqlQuery query(userdb);

    if (query.exec("SELECT id FROM MyFriend")) {
        while (query.next()) {
            friendIds.push_back(query.value(0).toInt());
        }
    } else {
        qDebug() << "Error while fetching friend IDs: " << query.lastError().text();
    }

    return friendIds;
}
