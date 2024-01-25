#include "leftbar.h"
//#include "titlebar.h"
#include "myconfig.h"
//#include "userinfownd.h"
//#include "sqlite/sqldatabase.h"

#include <QButtonGroup>
#include <QVBoxLayout>
#include <QPalette>
#include <QStringList>
#include <QDebug>
#include <QFileInfo>

#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif

LeftBar::LeftBar(int id,QWidget *parent) : QWidget(parent)
{
    setWindowFlag(Qt::FramelessWindowHint);
    setFixedWidth(55);

    QString headPath = MyConfig::strHeadPath + QString::number(id)+".jpg";
    QFileInfo fileInfo(headPath);
    if(!fileInfo.exists()){
        //MyConfig::strHeadFile = "default.png";
        //headPath = MyConfig::strHeadPath + MyConfig::strHeadFile;
        qDebug()<<"----------NOW ERROR OF LEFTBARRRRRRRRRRRR---------";
    }

    QStringList tmp;
    headLabel = new RoundLabel(this,headPath);
    //headLabel = new HeadLabel(this, headPath, ":/loginwnd/head_bkg_shadow", ":/loginwnd/head_bkg_highlight2",1);
    headLabel->setFixedSize(40, 40);
    headLabel->move(8,15);


    tmp << ":/Icons/MainWindow/chat.png"
        << ":/Icons/MainWindow/chat2.png"
        << ":/Icons/MainWindow/chat3.png";
    chatList = new MyButton(nullptr,tmp,QSize(30,30));
    chatList->setToolTip("聊天");
    tmp.clear();

    tmp << ":/Icons/MainWindow/contact.png"
        << ":/Icons/MainWindow/contact2.png"
        << ":/Icons/MainWindow/contact3.png";
    contacts = new MyButton(nullptr,tmp,QSize(30,30));
    contacts->setToolTip("联系人");
    tmp.clear();

    tmp << ":/Icons/MainWindow/settings.png"
        << ":/Icons/MainWindow/settings2.png"
        << ":/Icons/MainWindow/settings.png";
    settings = new MyButton(nullptr,tmp,QSize(30,30));
    settings->setToolTip("设置");
    tmp.clear();



    QVBoxLayout *layout = new QVBoxLayout(this);

    //layout->addWidget(headLabel,0,Qt::AlignCenter);
    layout->addStretch(2);
    layout->addWidget(chatList,0,Qt::AlignCenter);
    layout->addStretch(1);
    layout->addWidget(contacts,0,Qt::AlignCenter);
    layout->addStretch(1);
    layout->addWidget(settings,0,Qt::AlignCenter);
    layout->addStretch(15);


    m_btnGroup = new QButtonGroup(this);
    m_btnGroup->addButton(chatList, 0);
    m_btnGroup->addButton(contacts, 1);
    m_btnGroup->addButton(settings, 2);

    chatList->onBtnClicked();//默认选中会话列表按钮

    //原逻辑为HeadLabel，现在是RoundLabel
    //connect(headLabel,&RoundLabel::clicked,[&](){
        // qDebug() << "显示个人信息";
        // int id = MyApp::m_nId;
        // QString name = MyApp::m_strUserName;
        // QString head = MyApp::m_strHeadPath + MyApp::m_strHeadFile;

        // UserInfoWnd *myInfo = new UserInfoWnd(nullptr,id,name,head,0);

        // myInfo->show();
        // myInfo->move(QCursor::pos());
    //});

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(39, 41, 45)); // 使用 Window 而不是 Background
    setAutoFillBackground(true);
    setPalette(palette);

    setFocusPolicy(Qt::StrongFocus);
}

void LeftBar::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_WIN
    if (ReleaseCapture()) {
        QWidget *pWindow = this->window();
        if (pWindow->isWindow()) { // 这里使用 isWindow 替代 isTopLevel
            SendMessage(HWND(pWindow->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
        }
    }
    event->ignore();
#endif
}

