#include "contactslistwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QDebug>
#include <QButtonGroup>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>

#include "leftmenu/addsubgroup.h"
#include "sqlite/sqldatabase.h"
#include "clientapi/status.h"

ContactsListWidget::ContactsListWidget(QWidget *parent) : QListWidget(parent)
{

    setStyle();
    QStringList tmp;
    tmp << ":/Icons/MainWindow/friend.png"
        << ":/Icons/MainWindow/friend2.png"
        << ":/Icons/MainWindow/friend3.png";
    friendBtn = new MyButton(nullptr,tmp,QSize(20,20));
    friendBtn->setToolTip("好友");
    friendBtn->setStyleSheet("border: none;background-color:#ebeae8");
    tmp.clear();

    tmp << ":/Icons/MainWindow/group.png"
        << ":/Icons/MainWindow/group2.png"
        << ":/Icons/MainWindow/group3.png";
    groupBtn = new MyButton(nullptr,tmp,QSize(20,20));
    groupBtn->setToolTip("群聊");
    tmp.clear();

    btnGroup = new QButtonGroup(this);
    btnGroup->addButton(friendBtn, 0);
    friendBtn->onBtnClicked();
    btnGroup->addButton(groupBtn, 1);
    connect(btnGroup, &QButtonGroup::idClicked, this, &ContactsListWidget::onSwitchPage);

    QHBoxLayout *toplayout = new QHBoxLayout;
    toplayout->addWidget(friendBtn);
    toplayout->addWidget(groupBtn);

    stackWidget = new QStackedWidget;
    stackWidget->setContentsMargins(0,0,0,0);
    friendList = new ListWidget(nullptr,2);
    groupList = new ListWidget(nullptr,2);
    stackWidget->addWidget(friendList);
    stackWidget->addWidget(groupList);

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    //mainlayout->setContentsMargins(0,0,0,0);
    mainlayout->addLayout(toplayout);
    mainlayout->addWidget(stackWidget);


    stackWidget->setCurrentIndex(0);

    connect(friendList, &ListWidget::popMenuToShow, this, &ContactsListWidget::setPopMenuCell);
    connect(groupList, &ListWidget::popMenuToShow, this, &ContactsListWidget::setPopMenuCell);

    this->setContentsMargins(0,0,0,0);


}


void ContactsListWidget::InitList()
{
    //---------------------好友列表------------------------
    //设置好友列表抽屉右击菜单
    QMenu *dadMenu = new QMenu(this);
    dadMenu->addAction(tr("刷新"));
    dadMenu->addSeparator();
    dadMenu->addAction(tr("添加分组"));
    dadMenu->addAction(tr("删除该组"));
    QMenu *friendSortMenu = new QMenu(tr("排序"));
    dadMenu->addMenu(friendSortMenu);
    friendSortMenu->addAction(tr("按id排序"));
    friendSortMenu->addAction(tr("按名字排序"));

    connect(dadMenu, &QMenu::triggered, this, &ContactsListWidget::onFriendDadMenuSelected);

    friendList->setDadPopMenu(dadMenu);

    //设置好友列表中的好友格子右击菜单
    QMenu *sonMenu = new QMenu(this);
    sonMenu->addAction(tr("发消息"));
    sonMenu->addSeparator();
    sonMenu->addAction(tr("移动至黑名单"));
    sonMenu->addAction(tr("删除联系人"));
    QMenu *sonGroupListMenu = new QMenu(tr("移动联系人至"));
    sonMenu->addMenu(sonGroupListMenu);

    connect(sonMenu,SIGNAL(triggered(QAction*)),
            this,SLOT(onSonMenuSelected(QAction*)));
    friendList->setSonPopMenu(sonMenu);

    //查询本地数据库获取我的好友分组
    nlohmann::json mySubgroup = SqlDataBase::Instance()->getMySubgroup();

    qDebug() << "获取到的好友分组数量：" << mySubgroup.size(); // 打印分组数量


    for(size_t i = 0; i < mySubgroup.size(); i++){
        Cell *c = new Cell;
        auto &json = mySubgroup[i]; // 直接使用nlohmann::json对象，无需转换为QJsonObject

        c->type = Cell_FriendDrawer;
        c->groupName = QString::fromStdString(json["name"]); // 从nlohmann::json中获取字符串，并转换为QString
        c->id = 0;
        c->name = QString::fromStdString(json["name"]); // 同上
        c->isOpen = false;
        c->subTitle = QString("(0/0)");
        QList<Cell*> childs;
        c->childs = childs;

        hash.insert(c->groupName, c); // 建立名字到cell的映射
        friendList->insertCell(c);

        sonGroupListMenu->addAction(c->groupName);
        qDebug() << "已添加分组到UI：" << c->groupName; // 确认分组已被添加到UI

    }

    if (mySubgroup.empty()) {
        qDebug() << "未从数据库中获取到任何好友分组信息。";
    }

    // 查询本地数据库获取我的好友
    nlohmann::json myFriends = SqlDataBase::Instance()->getMyFriends();
    qDebug()<<"friends size: "<<myFriends.size();
    for (size_t i = 0; i < myFriends.size(); i++) {
        auto &json = myFriends[i]; // 直接使用nlohmann::json对象
        Cell *c = new Cell;
        c->id = json["id"]; // 直接从nlohmann::json获取值
        c->name = QString::fromStdString(json["name"]); // 将std::string转换为QString
        c->iconPath = QString::fromStdString(json["head"]); // 同上
        c->groupName = QString::fromStdString(json["subgroup"]); // 同上
        c->type = Cell_FriendContact;
        c->status = Status::OnLine; //不该这么武断

        QFileInfo fileInfo(c->iconPath);
        if (c->iconPath.isEmpty() || !fileInfo.exists()) {
            qDebug() << c->iconPath << "头像文件不存在，正在向服务器获取...";

            // 假设有逻辑向服务器请求图片

            //MyConfig::Sleep(500); // 等待500毫秒  待完成

            QString headPath = MyConfig::strHeadPath + QString::number(c->id) + ".jpg";
            QFileInfo fileInfo_(headPath);
            if (!fileInfo_.exists()) {
                c->iconPath = ":/Icons/MainWindow/default_head_icon.png"; // 没有收到则显示默认头像
            } else {
                c->iconPath = headPath;

                // 更新本地数据库
                QSqlQuery query;
                QString strSql = QString("UPDATE MyFriend SET head='%1' WHERE id=%2").arg(headPath, QString::number(c->id));
                query.exec(strSql);
            }
        }
        // 添加至相应的抽屉下
        hash[c->groupName]->childs.append(c);


    }

    friendList->refreshList();

    connect(friendList, &ListWidget::sonDoubleClicked,
            this, &ContactsListWidget::onSonDoubleClicked);


    //------------------------群列表------------------------
    //设置群列表抽屉右击菜单
    QMenu *groupDadMenu = new QMenu(this);
    groupDadMenu->addAction(tr("刷新"));
    QMenu *groupSortMenu = new QMenu(tr("排序"));
    groupDadMenu->addMenu(groupSortMenu);
    groupSortMenu->addAction(tr("按id排序"));
    groupSortMenu->addAction(tr("按名字排序"));


    connect(groupDadMenu, &QMenu::triggered,
            this, &ContactsListWidget::onGroupDadMenuSelected);
    groupList->setDadPopMenu(groupDadMenu);

    //设置群列表中的群格子右击菜单
    QMenu *groupSonMenu = new QMenu(this);
    groupSonMenu->addAction(tr("发送群消息"));
    groupSonMenu->addAction(tr("退出该群"));
    groupList->setSonPopMenu(groupSonMenu);
    connect(groupSonMenu, &QMenu::triggered,
            this, &ContactsListWidget::onSonMenuSelected);

    //添加默认项
    myGroup = new Cell;
    myGroup->groupName = QString(tr("我的群组"));
    myGroup->isOpen = false;
    myGroup->type = Cell_GroupDrawer;
    myGroup->id = 0;
    myGroup->name = QString(tr("我的群组"));
    myGroup->subTitle = QString("(0/0)");
    groupList->insertCell(myGroup);
    connect(groupList, &ListWidget::sonDoubleClicked,
            this, &ContactsListWidget::onSonDoubleClicked);

    QList<Cell*> childs;
    myGroup->childs = childs;

    // 查询本地数据库获取我的群
    nlohmann::json myGroups = SqlDataBase::Instance()->getMyGroups();
    for (size_t i = 0; i < myGroups.size(); i++) {
        auto &json = myGroups[i]; // 直接使用nlohmann::json对象
        Cell *c = new Cell;
        c->id = json["id"]; // 直接从nlohmann::json获取值
        c->name = QString::fromStdString(json["name"]); // 将std::string转换为QString
        c->groupName = "我的群组";
        c->iconPath = QString::fromStdString(json["head"]); // 同上
        c->type = Cell_GroupContact;
        c->status = Status::OnLine;

        QFileInfo fileInfo(c->iconPath);
        if (c->iconPath.isEmpty() || !fileInfo.exists()) {
            qDebug() << c->iconPath << "头像文件不存在，正在向服务器获取...";
            // 向服务器请求图片

            MyConfig::Sleep(500); // 等待半秒(仿佛真的有请求一样）

            QString headPath = MyConfig::strHeadPath + QString::number(c->id) + ".jpg";
            QFileInfo fileInfo_(headPath);
            if (!fileInfo_.exists()) {
                c->iconPath = ":/Icons/MainWindow/default_head_icon.png"; // 没有收到则显示默认头像
            } else {
                c->iconPath = headPath;

                // 更新本地数据库
                QSqlQuery query;
                QString strSql = QString("UPDATE MyGroup SET head='%1' WHERE id=%2").arg(headPath, QString::number(c->id));
                query.exec(strSql);
            }
        }

        myGroup->childs.append(c); // 添加至相应的群组下
    }
    groupList->refreshList();
}

void ContactsListWidget::addCell(Cell *c)
{
    if(c->type == Cell_FriendContact){
        friendList->insertCell(c);
        friendList->refreshList();
    }else if(c->type == Cell_GroupContact){
        groupList->insertCell(c);
        groupList->refreshList();
    }
}

void ContactsListWidget::onFriendDadMenuSelected(QAction *action) {
    QString actionText = action->text();
    qDebug() << actionText;

    if(actionText == tr("添加分组")) {
        qDebug() << "添加分组";
        AddSubGroup w;
        connect(&w, &AddSubGroup::updateList, this, &ContactsListWidget::sltUpdateFriendList);
        w.exec();
    } else if(actionText == tr("刷新")) {
        qDebug() << "刷新";
        //refreshFriendList(); // 待定-假设有一个方法来刷新好友列表
    } else if(actionText == tr("删除该组")) {
        qDebug() << "删除该组";
        if(popMenuCell->groupName == tr("默认分组") || popMenuCell->groupName == tr("黑名单")) {
            QMessageBox::information(this, "错误", QString("<%1>不可删除!").arg(popMenuCell->groupName));
            return;
        }

        // 删除分组并更新相关好友的分组为“默认分组”
        QSqlQuery query;
        query.prepare("UPDATE MyFriend SET subgroup='默认分组' WHERE subgroup=:groupName");
        query.bindValue(":groupName", popMenuCell->groupName);
        query.exec();

        query.prepare("DELETE FROM MySubgroup WHERE name=:groupName");
        query.bindValue(":groupName", popMenuCell->groupName);
        query.exec();

        friendList->RemoveCell(popMenuCell); // 界面上移除该分组并刷新列表

    } else if(actionText == tr("按id排序") || actionText == tr("按名字排序")) {
        // 假设sortById() 和 sortByName() 是Cell的成员函数
        if(actionText == tr("按id排序")) popMenuCell->sortById();
        else popMenuCell->sortByName();
        friendList->refreshList(); // 刷新列表显示排序结果
    }

    if(popMenuCell != nullptr) {
        qDebug() << "当前右击的Cell是:" << popMenuCell->id << popMenuCell->name;
    }
}

void ContactsListWidget::onGroupDadMenuSelected(QAction *action)
{
    qDebug() << action->text();
    if(!action->text().compare(tr("刷新"))){
    }else if(!action->text().compare(tr("按id排序"))){
        popMenuCell->sortById();
        groupList->refreshList();
    }else if(!action->text().compare(tr("按名字排序"))){
        popMenuCell->sortByName();
        groupList->refreshList();
    }

    if(popMenuCell != nullptr){
        qDebug() << "当前右击的Cell是:" << popMenuCell->id << popMenuCell->name;
    }
}

void ContactsListWidget::onSonMenuSelected(QAction *action)
{
    qDebug() << action->text();
    if (!action->text().compare(tr("发消息")) || !action->text().compare(tr("发送群消息"))) {
        emit openDialog(popMenuCell);
    } else if (!action->text().compare(tr("移动至黑名单"))) {
        moveContactToGroup("黑名单");
    } else if (!action->text().compare(tr("删除联系人"))) {
        deleteContact();
    } else if (!action->text().compare(tr("退出该群"))) {
        leaveGroup();
    } else {
        // 更改联系人分组
        moveContactToGroup(action->text());
    }

    if (popMenuCell != nullptr) {
        qDebug() << "当前右击的Cell是:" << popMenuCell->id << popMenuCell->name;
    }
}


void ContactsListWidget::setStyle()
{
    QString styleSheet = R"(
        ContactsListWidget {
            border: none;
            background-color: rgb(235, 234, 232);
        }

        ContactsListWidget::item {
            border-bottom: 1px solid lightgray;
            background-color: rgb(235, 234, 232);
        }

        ContactsListWidget::item:selected {
            background-color: lightblue;
        }

        ContactsListWidget::branch {
            background: palette(base);
        }
    )";

    setStyleSheet(styleSheet);

}

void ContactsListWidget::moveContactToGroup(const QString& groupName)
{
if (groupName == popMenuCell->groupName) return; // 如果已在该分组，无需操作

    if (groupName == tr("黑名单") && (popMenuCell->name == tr("默认分组") || popMenuCell->name == tr("黑名单"))) {
        QMessageBox::information(this, "错误", QString("<%1>不可进行此操作!").arg(popMenuCell->name));
        return;
    }

    // 执行移动操作...
    friendList->RemoveCell(popMenuCell);
    popMenuCell->groupName = groupName;
    friendList->insertCell(popMenuCell);

    updateContactGroupInDatabase(popMenuCell->id, groupName);
}

void ContactsListWidget::updateContactGroupInDatabase(int contactId, const QString& newGroupName) {
    QSqlQuery query;
    query.prepare("UPDATE MyFriend SET subgroup=:groupName WHERE id=:id");
    query.bindValue(":groupName", newGroupName);
    query.bindValue(":id", contactId);
    if (!query.exec()) {
        qDebug() << "更新联系人分组信息失败 updateContactGroupInDatabase";
    }
}

//删除联系人
void ContactsListWidget::deleteContact() {
    if (popMenuCell == nullptr) {
        qDebug() << "No cell selected for deletion.";
        return;
    }

    // 更新本地数据库
    QSqlQuery query;
    query.prepare("DELETE FROM MyFriend WHERE id = :id");
    query.bindValue(":id", popMenuCell->id);
    if (!query.exec()) {
        qDebug() << "删除联系人失败：";
    }

    // 删除联系人列表中的该格子
    friendList->RemoveCell(popMenuCell);

    // 从UI中删除对应的聊天框
    emit deleteChat(popMenuCell->id);

    /*
    // 可能需要稍微延时以确保UI更新，这里采用QTimer::singleShot而非阻塞式Sleep
    QTimer::singleShot(100, this, [this]() {
        // 通知服务器，删除好友
        QJsonObject json;
        json.insert("userID1", MyConfig::m_nId); // 我的id
        json.insert("userID2", popMenuCell->id); // 该好友的id
        emit signalSendMessage(DeleteFriend, json);
    });
*/
    // 最后清除popMenuCell指针或做其他必要的清理工作
    popMenuCell = nullptr; // 如果popMenuCell需要在这之后被使用，请相应地处理，避免悬挂指针问题

}

// 示例：离开群组
void ContactsListWidget::leaveGroup() {
    // 特定逻辑，比如检查是否是群主...
}

void ContactsListWidget::displayNewGroups(int id)
{
    // 查询本地数据库获取新建的群信息
    nlohmann::json groupInfo = SqlDataBase::Instance()->getGroupInfo(id);

    qDebug()<<QString::fromStdString(groupInfo.dump());
    if (!myGroup) {
        qDebug() << "找不到“我的群组”项，无法显示新建的群信息。";
        return;
    }

    qDebug()<<"x1";
    // 将新建的群信息显示在界面中
    Cell *c = new Cell;
    c->id = id;
    c->name = QString::fromStdString(groupInfo["name"]);
    qDebug()<<"x2";
    c->groupName = tr("我的群组");
    c->iconPath = QString::fromStdString(groupInfo["head"]);
    qDebug()<<"x3";
    c->type = Cell_GroupContact;
    c->status = Status::OnLine;

    QFileInfo fileInfo(c->iconPath);
    if (c->iconPath.isEmpty() || !fileInfo.exists()) {
        // 处理头像文件不存在的情况
        qDebug() << c->iconPath << "头像文件不存在，正在向服务器获取...";

        // 更新本地数据库中的头像路径
        // QSqlQuery query;
        // QString strSql = QString("UPDATE MyGroup SET head='%1' WHERE id=%2").arg(headPath, QString::number(c->id));
        // query.exec(strSql);

        c->iconPath = ":/Icons/MainWindow/default_head_icon.png"; // 显示默认头像
    }

    myGroup->childs.append(c); // 添加至相应的群组下

    // 更新 UI
    groupList->refreshList();
}

void ContactsListWidget::onSonDoubleClicked(Cell *cell)
{
    //打开私聊或者群聊窗口
    if(cell->type == Cell_FriendContact){
        qDebug() << "打开私聊窗口: " << cell->id;
        //emit openDialog(cell);
        emit checkinContact(cell);
    }else if(cell->type == Cell_GroupContact){
        qDebug() << "打开群聊窗口: " << cell->id;
        //    emit openDialog(cell);
        emit checkinContact(cell);
    }
}

void ContactsListWidget::setPopMenuCell(Cell *cell, QMenu *)
{
    qDebug() << "popMenu show on cell:" << cell->id << cell->name;
    popMenuCell = cell;
}

void ContactsListWidget::sltUpdateFriendList(QString name)
{
    Cell *c = new Cell;
    c->type = Cell_FriendDrawer;
    c->groupName = name;
    c->name = name;
    c->isOpen = false;
    c->subTitle = QString("(0/0)");
    QList<Cell*> childs;
    c->childs = childs;

    hash.insert(c->groupName,c);//建立名字到cell的映射
    friendList->insertCell(c);
}

void ContactsListWidget::onSwitchPage(int page)
{
    if(page == 0){
        //qDebug() << "显示好友列表";
        friendBtn->onBtnClicked();
        groupBtn->restoreBtn();
    }else if(page == 1){
        //qDebug() << "显示群列表";
        friendBtn->restoreBtn();
        groupBtn->onBtnClicked();
    }

    stackWidget->setCurrentIndex(page);
}
