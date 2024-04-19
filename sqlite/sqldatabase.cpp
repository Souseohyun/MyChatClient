#include "sqldatabase.h"

#include "chatsession/cell.h"


#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QFile>
#include <QSqlError>
#include <QJsonArray>
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

    qDebug() << "Database path:" << dataName;

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

    query.exec(QString("insert into MySubgroup values('我的好友','") + DATE_TME_FORMAT + "')");
    query.exec(QString("insert into MySubgroup values('黑名单','") + DATE_TME_FORMAT + "')");

    // 创建历史聊天表 tag = 0表示私聊消息,tag = 1表示群聊消息,
    // sender = 0表示是我发的消息,sender = 1表示是对方发的消息,sender = 2表示是系统发的消息
    query.exec("CREATE TABLE MsgHistory (id INT PRIMARY KEY, sender INT, "
               "myID INT, yourID INT, groupID INT, tag bit, "
               "type INT, content varchar(500), filesize varchar(30), download bit, "
               "datetime BIGINT)");


    qDebug() << "Database opened for user" << userId << "with connection name:" << connectionName;

    return true;
}

void SqlDataBase::addHistoryMsg(BubbleInfo *info) {
    // 查询最新的消息ID
    QSqlQuery query(userdb);
    query.exec("SELECT id FROM MsgHistory ORDER BY id DESC;");
    int maxId = 0; // 消息序号,为该表的主键
    if (query.next()) {
        maxId = query.value(0).toInt(); // 获取当前最高ID
    }

    // 准备插入新的消息记录
    query.prepare("INSERT INTO MsgHistory (id, sender, myID, yourID, groupID, tag, "
                  "type, content, filesize, download, datetime) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    query.bindValue(0, maxId + 1); // 自增ID
    query.bindValue(1, info->srcid); // 发送者ID
    query.bindValue(2, info->myid); // 我的ID
    query.bindValue(3, info->destid); // 到达者ID
    query.bindValue(4, 0/*info->groupID*/); // 待完成，群组ID，若无则为0
    query.bindValue(5, 0/*info->tag*/); // 消息标签
    query.bindValue(6, 0/*info->msgType*/); // 消息类型
    query.bindValue(7, info->message); // 消息内容
    query.bindValue(8, 0/*info->fileSize*/); // 文件大小，若无文件则为0
    query.bindValue(9, 0/*info->downloaded*/); // 文件是否已下载
    query.bindValue(10, info->time); // 消息发送时间

    if (!query.exec()) {
        qDebug() << "添加历史消息失败: " << query.lastError().text();
    }
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

void SqlDataBase::createGroup(int groupId, const QString &groupName, int adminId, const QString &headImage)
{
    // 检查是否已经存在该群聊
    QString strQuery = QString("SELECT * FROM MyGroup WHERE id = %1").arg(groupId);
    QSqlQuery query(userdb);
    query.exec(strQuery);

    if (query.next()) {
        // 查询到有该群聊，不添加
        qDebug() << "群聊已存在，无法再次创建: " << groupId;
        return;
    }

    // 插入新的群聊记录
    query.prepare("INSERT INTO MyGroup (id, name, admin, head, tag, lastMsg, lastTime) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?);");
    query.bindValue(0, groupId);
    query.bindValue(1, groupName);
    query.bindValue(2, adminId);
    query.bindValue(3, headImage);
    query.bindValue(4, false); // tag 初始值为 false
    query.bindValue(5, ""); // lastMsg 初始值为空
    query.bindValue(6, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")); // 当前时间为最后消息时间

    if (!query.exec()) {
        qDebug() << "创建群聊失败: " << query.lastError().text();
    }
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

    QString strQuery = "SELECT * FROM MyFriend";
    QSqlQuery query(userdb);            //一定要指定userdb，否则可能会not open db
    query.prepare(strQuery);
    int i = 0;
    //成年老bug：query.exec是必须的啊，不是直接while query.next就可以了。
    if (!query.exec()) {
        qDebug() << "查询失败：" << query.lastError().text();
                                         return nullptr; // 或处理错误
    }
    while (query.next()){
        qDebug()<<"i = "<<++i;
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

nlohmann::json SqlDataBase::getMyGroups() const
{
    using json = nlohmann::json;

    json myGroups = json::array(); // 使用nlohmann::json定义一个JSON数组

    QString strQuery = "SELECT * FROM MyGroup";
    QSqlQuery query(userdb);
    query.prepare(strQuery);
    if (!query.exec()) {
        qDebug() << "查询失败：" << query.lastError().text();
                                         return nullptr; // 或处理错误
    }
    while (query.next()) {
        json group;
        group["id"] = query.value(0).toInt();
        group["name"] = query.value(1).toString().toStdString(); // 将QString转换为std::string
        group["admin"] = query.value(2).toInt();
        group["head"] = query.value(3).toString().toStdString(); // 同上
        myGroups.push_back(group); // 将json对象添加到数组中
    }

    return myGroups; // 返回nlohmann::json类型的数组
}

nlohmann::json SqlDataBase::getMySubgroup() const
{
    using json = nlohmann::json;

    json myGroup = json::array(); // 使用nlohmann::json定义一个JSON数组

    QString strQuery = "SELECT * FROM MySubgroup";
    QSqlQuery query(userdb); // 指定特定的数据库连接，否则可能not open db
    query.prepare(strQuery);
    if (!query.exec()) {
        qDebug() << "查询失败：" << query.lastError().text();
        return nullptr; // 或处理错误
    }

    while(query.next()){
        json jsonGroup;
        jsonGroup["name"] = query.value(0).toString().toStdString(); // 将QString转换为std::string
        myGroup.push_back(jsonGroup); // 将json对象添加到数组中
    }

    return myGroup; // 返回nlohmann::json类型的数组
}

nlohmann::json SqlDataBase::getGroupInfo(int id) const
{
    QString strQuery = "SELECT * FROM MyGroup where id = ";
    strQuery.append(QString::number(id));
    QSqlQuery query(userdb);
    query.prepare(strQuery);
    //QSqlQuery query(strQuery);
    nlohmann::json json;

    if (!query.exec()) {
        qDebug() << "查询失败：" << query.lastError().text();
                                         return nullptr; // 或处理错误
    }
    while (query.next()) {
        json["name"] = query.value(1).toString().toStdString();
        json["head"] = query.value(3).toString().toStdString();
    }
    return json;
}

bool SqlDataBase::isMyFriend(const int &friendID){
    QString strQuery = "SELECT * FROM MyFriend WHERE id = :friendID";
    QSqlQuery query(userdb);
    query.prepare(strQuery);
    query.bindValue(":friendID", friendID);

    if (!query.exec()) {
        qDebug() << "查询失败：" << query.lastError().text();
        return false; // 或其他错误处理
    }

    return query.next(); // 如果查询有结果，则表示是好友
}

bool SqlDataBase::isInGroup(const int &groupID){
    QString strQuery = "SELECT * FROM MyGroup WHERE id = :groupID";
    QSqlQuery query(userdb);
    query.prepare(strQuery);
    query.bindValue(":groupID", groupID);

    if (!query.exec()) {
        qDebug() << "查询失败：" << query.lastError().text();
        return false; // 或其他错误处理
    }

    return query.next(); // 如果查询有结果，则表示在群组中
}

QVector<BubbleInfo *> SqlDataBase::QueryMsgHistory(int id, int tag, int count)
{
    qDebug()<<"__DEBUG:INTO HISTORY";
    QString queryString = QString("SELECT * FROM MsgHistory");
    if(tag == 0){
        queryString.append(" WHERE (yourID="); // 查询发送给id的消息
        queryString.append(QString::number(id));
        queryString.append(" OR sender="); // 或者由id发送的消息
        queryString.append(QString::number(id));
        queryString.append(") and tag=");
        queryString.append(QString::number(tag));
        queryString.append(" ORDER BY id ASC;");
    }else if(tag == 1){
        queryString.append(" WHERE groupID=");
        queryString.append(QString::number(id));
        queryString.append(" and tag=");
        queryString.append(QString::number(tag));
        queryString.append(" ORDER BY id ASC;");
    }

    //当前只能无脑加载全程，往上翻时累计查10条 这种功能待完成
    QVector<BubbleInfo*> bubbles;

    QSqlQuery query(userdb);
    if (!query.exec(queryString)) {
        qDebug() << "查询历史消息失败: " << query.lastError().text();
        return bubbles; // 返回空
    }
    while(query.next()){
    //(id INT PRIMARY KEY, sender INT, myID INT, yourID INT, groupID INT, tag bit, type INT,
    // content varchar(500), filesize varchar(30), download bit, datetime BIGINT);
    //1|1003|1003|1004|0|0|0|marry me ! <img src=":/emoji/kl.png" />|0|0|1710345242
        BubbleInfo *bubble = new BubbleInfo;
        bubble->srcid = query.value(1).toInt();
        bubble->myid = query.value(2).toInt();
        bubble->destid = query.value(3).toInt();
        bubble->isGroup = (query.value(4).toInt() == 0)? false : true;
        //tag没有
        //type没有
        bubble->message = query.value(7).toString();
        //filesize没有
        //download tag 没有
        bubble->time = query.value(10).toInt();

        bubbles.push_back(bubble);
    }
    return bubbles;
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
