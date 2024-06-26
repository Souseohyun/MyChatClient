#include "addfriendwnd.h"


#include "sqlite/sqldatabase.h"
#include "myconfig.h"

#include <QSqlQuery>
#include <QJsonObject>
#include <QDateTime>

AddFriendWnd::AddFriendWnd(Cell *cell)
{
    setFixedSize(400,200);
    setWindowFlags(Qt::WindowCloseButtonHint);

    this->cell = cell;

    if(cell->type == Cell_AddFriend)
        setWindowTitle("申请添加好友");
    else if(cell->type == Cell_AddGroup)
        setWindowTitle("申请加入群：" + QString::number(cell->groupid));
    font = QFont("Microsoft YaHei", 12, 50, false);
    headLabel = new RoundLabel(this,cell->iconPath);
    headLabel->setFixedSize(80,80);

    idLabel = new QLabel(QString::number(cell->id),this);
    idLabel->setFont(font);
    nameLabel = new QLabel(cell->name,this);
    nameLabel->setFont(font);

    bool tag = cell->done;

    if(tag){
        noticeLabel = new QLabel("已处理该请求",this);
        noticeLabel->setFont(font);
        noticeLabel->setStyleSheet("color:#0081ef");
        noticeLabel->setGeometry(135,150,200,30);
    }

    agreeBtn = new QPushButton("同意",this);
    rejectBtn = new QPushButton("拒绝",this);

    headLabel->setGeometry(120,40,80,80);
    idLabel->setGeometry(210,45,150,30);
    nameLabel->setGeometry(210,85,150,30);

    agreeBtn->setGeometry(290,150,80,30);
    agreeBtn->setStyleSheet("QPushButton{border:1px solid #86949e;background-color:#0188fb;border-radius:5px}"
                            "QPushButton:hover{background-color:#289cff;}"
                            "QPushButton:pressed{background-color:#0081ef}");
    rejectBtn->setGeometry(190,150,80,30);
    rejectBtn->setStyleSheet("QPushButton{border:1px solid #86949e;background-color:#0188fb;border-radius:5px}"
                             "QPushButton:hover{background-color:#289cff;}"
                             "QPushButton:pressed{background-color:#0081ef}");
    connect(agreeBtn,SIGNAL(clicked(bool)),this,SLOT(sltBtnClicked()));
    connect(rejectBtn,SIGNAL(clicked(bool)),this,SLOT(sltBtnClicked()));
    if(tag){
        agreeBtn->setVisible(false);
        rejectBtn->setVisible(false);
    }

    QPalette palette;
    palette.setColor(QPalette::Window, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(palette);
}

//乙方同意后，好友加入乙方的数据库中
void AddFriendWnd::sltBtnClicked()
{
    if(sender() == agreeBtn){
        if(cell->type == Cell_AddFriend){
            cell->done = true;

            //更新本地服务器
            SqlDataBase::Instance()->addFriend(MyConfig::userId,cell->id,cell->name,"我的好友",cell->markname);


            //通知服务器，已同意
            nlohmann::json json;
            json["type"] = "result_friend";
            json["isadd"]= true;
            json["srcid"]= MyConfig::userId;
            json["addid"]= cell->id;

            emit signalAddFriend2(json);

            //通知中栏添加与该用户的聊天格子
            Cell *c = new Cell;
            c->type = Cell_FriendChat;
            c->id = cell->id;
            c->name = cell->name;
            c->markname = cell->markname;
            c->iconPath = cell->iconPath;
            c->isClicked = false;
            c->subTitle = QDateTime::currentDateTime().toString("hh:mm:ss");
            c->msg = "你和" + QString::number(c->id) + "已经成为好友了！";
            c->status = Status::OnLine;
            //告诉clintwindow，更新一下会话栏
            emit signalAddChat(c);

        }else if(cell->type == Cell_AddGroup){
            cell->done = true;

            //通知服务器，已同意
            QJsonObject json;
            json.insert("groupID",cell->groupid);
            json.insert("userID",cell->id);
            json.insert("time",QDateTime::currentDateTime().toString("yyyy-MM-dd  hh:mm:ss"));
            emit signalAddFriend(AddGroupRequist,json);

            MyConfig::Sleep(100);

            //通知中栏添加与该群的聊天格子(如果已存在则只更新格子)
            Cell *c = new Cell;
            c->type = Cell_GroupChat;
            c->id = cell->groupid;//格子的id为群id
            c->name = cell->groupname_;
            c->iconPath = cell->groupHead;
            c->isClicked = false;
            c->subTitle = QDateTime::currentDateTime().toString("hh:mm:ss");
            c->msg = "用户" + QString::number(cell->id) + "已经加入该群";
            c->status = Status::OnLine;

            //用于刷新该群的群员列表
            c->newUserID = cell->id;
            c->newUserName = cell->name;
            c->newUserHead = cell->iconPath;

            emit signalAddChat(c);
        }

        agreeBtn->setVisible(false);
        rejectBtn->setVisible(false);

        noticeLabel = new QLabel("已同意请求",this);
        noticeLabel->setStyleSheet("color:#0081ef");
        noticeLabel->setFont(font);
        noticeLabel->setVisible(true);
        noticeLabel->setGeometry(150,150,200,30);
    }else{
        this->close();
    }
}

