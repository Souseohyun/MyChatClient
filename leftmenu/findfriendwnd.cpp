#include "findfriendwnd.h"
#include "sqlite/sqldatabase.h"
#include "clientapi/myapplication.h"
#include "clientapi/status.h"
#include "myconfig.h"


#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QJsonObject>
#include <QDebug>
#include <QFileInfo>

FindFriendWnd::FindFriendWnd(NetworkManager& net,int tag)
    : QDialog(),tag(tag),imageNet_(net)
{
    setFixedSize(700,250);
    setWindowFlags(Qt::WindowCloseButtonHint);

    QLabel *label = new QLabel(this);
    if(tag == 0)
        label->setText("找好友");
    else if(tag == 1)
        label->setText("找群");
    QFont font = QFont("Microsoft YaHei", 18, 50, false);
    QFont labelFont = QFont("Microsoft YaHei", 12, 50, false);
    QFont resultFont = QFont("Microsoft YaHei", 14, 50, false);
    label->setFont(font);

    label->setGeometry(300,30,100,30);

    searchBar = new SearchBar(this,QSize(500,50),1);
    searchBar->setGeometry(40,80,500,50);

    QPushButton *searchBtn = new QPushButton("搜索",this);
    searchBtn->setGeometry(560,85,100,40);
    searchBtn->setStyleSheet("QPushButton{border:1px solid #86949e;background-color:#0188fb;border-radius:5px}"
                             "QPushButton:hover{background-color:#289cff;}"
                             "QPushButton:pressed{background-color:#0081ef}");
    //connect(searchBtn,SIGNAL(clicked(bool)),this,SLOT(sltBtnClicked()));
    connect(searchBtn,&QPushButton::clicked,this,&FindFriendWnd::sltBtnClicked);
    this->setAutoFillBackground(true); // 保留这句以确保背景填充
    this->setStyleSheet("background-color: white;"); // 使用样式表设置背景色




    resultLabel = new QLabel(this);
    resultLabel->setVisible(false);
    resultLabel->setGeometry(270,160,250,30);
    resultLabel->setFont(resultFont);

    idLabel = new QLabel(this);
    idLabel->setVisible(false);
    idLabel->setFont(labelFont);
    idLabel->setFixedSize(80,30);
    idLabel->move(370,250);

    nameLabel = new QLabel(this);
    nameLabel->setVisible(false);
    nameLabel->setFont(labelFont);
    nameLabel->setFixedSize(80,30);
    nameLabel->move(370,280);

    headLabel = new RoundLabel(this);
    headLabel->setVisible(false);
    headLabel->setFont(labelFont);
    headLabel->setFixedSize(100,100);
    headLabel->move(250,230);

    addBtn = new QPushButton(this);
    addBtn->setGeometry(300,350,80,30);
    addBtn->setStyleSheet("QPushButton{border:1px solid #86949e;background-color:#0188fb;border-radius:5px}"
                          "QPushButton:hover{background-color:#289cff;}"
                          "QPushButton:pressed{background-color:#0081ef}");
    addBtn->setVisible(false);
    //connect(addBtn,SIGNAL(clicked(bool)),this,SLOT(sltAddFriend()));
    connect(addBtn,&QPushButton::clicked,this,&FindFriendWnd::sltAddFriend);
    msgLabel = new QLabel("已发送请求！",this);
    msgLabel->setFont(resultFont);
    msgLabel->setGeometry(280,370,150,30);
    msgLabel->setStyleSheet("color:#0081ef");
    msgLabel->setVisible(false);

    this->setFocusPolicy(Qt::StrongFocus);
    this->setStyleSheet("QDialog{border-radius:5px}");
}

//该函数仅负责搜索框低级逻辑，不包含数据库查找等关键代码
void FindFriendWnd::sltBtnClicked()
{
    //点击搜索的btn进入这里
    this->setFixedSize(700,500);

    //MyConfig::FormInCenter(this);     //窗口居中显示

    QString text = searchBar->text();
    int cnt = text.size();
    for(int i = 0;i < cnt;i++){
        if(text[i] >= '0' && text[i] <= '9'){
            continue;
        }else{
            QMessageBox::information(this,"错误","用户id包含非法字符");
            return;
        }
    }

    resultLabel->setVisible(false);
    idLabel->setVisible(false);
    nameLabel->setVisible(false);
    headLabel->setVisible(false);
    addBtn->setVisible(false);
    msgLabel->setVisible(false);

    int id = text.toInt();
    nlohmann::json json;
    json["tag"] = tag;
    json["id"] = id;

    //该信号发给clientwindow，由它拿着信息向它的成员 服务器管理者请求查询
    emit signalFind(json);
}

//该函数仅作用于已经在数据库中找到结果后的reply，并不是查找过程噢
void FindFriendWnd::sltFindFriendReply(const nlohmann::json &json)
{
    std::string str = json.value("type","");
    QString qs = QString::fromStdString(str);
    qDebug()<<"search result 顺利抵达槽函数 "<<qs;

    if(true){

        bool isFind = json.value("isfind",false);
        if(!isFind){
            if(tag == 0)
                resultLabel->setText("没有查询到该用户");
            else if(tag == 1)
                resultLabel->setText("没有查询到该群");
            resultLabel->show();
        }else{
            int id = json.value("userid",-1);
            int gender = json.value("gender",-1);
            QString name = QString::fromStdString(json.value("username",""));
            QString head = QString::fromStdString(json.value("avaurl",""));

            //nonono,这里需要向服务器请求图片
            std::string http = imageNet_.as_HttpSearchImageByUserId(id);
            imageNet_.SendToImageServer(http);
            MyConfig::Sleep(500);//等待半秒
            QString filename = QString("%1.jpg").arg(id);
            QString fullPath = MyConfig::strHeadPath + filename;

            QFileInfo fileInfo(fullPath);

            if(!fileInfo.exists()){


                    head = ":/Icons/MainWindow/default_head_icon.png";//没有收到则显示默认头像
                }else{
                    head = fullPath;

                }

                head = MyConfig::strHeadPath + filename;



            resultLabel->setText("查询到下列信息");
            resultLabel->setVisible(true);
            idLabel->setText(QString::number(id));
            idLabel->setVisible(true);
            nameLabel->setText(name);
            nameLabel->setVisible(true);
            qDebug() << "head path:" << head;
            headLabel->setPixmap(head);
            headLabel->setVisible(true);
            if(tag == 0)
                addBtn->setText("添加好友");
            else if(tag == 1)
                addBtn->setText("加入该群");
            addBtn->setVisible(true);

            friendID = id;
            }
    }

}

//这是添加好友的关键代码，点击addbtn后触发，但点击搜索后的槽函数可不是这
void FindFriendWnd::sltAddFriend()
{
    qDebug()<<"_______DEBUG:INTO SLT ADD FRIEND";
    nlohmann::json json;
    json["type"] = "add_friend";
    json["isfriend"] = true;
    json["srcid"] = MyConfig::userId;
    //name等信息交由服务器端数据库拿着srcid查询，不进行网络通信
    json["addid"]=friendID;

    if(tag == 0){
        if(friendID == MyConfig::userId){
            QMessageBox::information(this,"错误","无法添加自己为好友");
        }else{
            bool flag = SqlDataBase::Instance()->isMyFriend(friendID);
            if(flag){
                QMessageBox::information(this,"错误","您和该用户已经是好友了");
            }else{
                msgLabel->setVisible(true);
                addBtn->setVisible(false);
                //待修改，发送好友请求并与服务器交互 log by 2024.3.12
                emit signalAddFriend(json);
            }
        }
    }else if(tag == 1){
        bool flag = SqlDataBase::Instance()->isInGroup(friendID);
        if(flag){
            QMessageBox::information(this,"错误","您已经加入该群了");
        }else{
            msgLabel->setVisible(true);
            addBtn->setVisible(false);
            json["isfriend"] = false;
            emit signalAddFriend(json);
        }
    }
}
