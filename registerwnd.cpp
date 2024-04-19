#include "RegisterWnd.h"
#include "myconfig.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>


RegisterWndPrivate::RegisterWndPrivate(QWidget *parent)
    : MyWindow(parent)
{
    QFont font = QFont("Microsoft YaHei", 18, 50, false);
    QFont font2 = QFont("Microsoft YaHei", 12, 48, false);


    this->setFixedSize(430,330);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    m_notifyMsg= new QLabel(this);
    m_notifyMsg->setFixedSize(430, 20);
    m_notifyMsg->setStyleSheet("background-color:#09a3dc;font-size: 12px;font-family:Microsoft YaHei;");
    m_notifyMsg->hide();

    m_menuCloseBtn = new QPushButton(this);
    m_menuCloseBtn->setFlat(true);
    m_menuCloseBtn->setFixedSize(30, 32);
    m_menuCloseBtn->move(400, 0);
    m_menuCloseBtn->setStyleSheet("QPushButton{ border-image: url(:/login/close_normal); }"
                                  "QPushButton:hover:!pressed{ border-image: url(:/login/close_hover); }"
                                  "QPushButton:hover:pressed{ border-image: url(:/login/close_press); border-style:none; }");
    connect(m_menuCloseBtn, &QPushButton::pressed, this, &RegisterWndPrivate::closeWnd);

    m_menuMinBtn = new QPushButton(this);
    m_menuMinBtn->setFlat(true);
    m_menuMinBtn->setFixedSize(30, 32);
    m_menuMinBtn->move(370, 0);
    m_menuMinBtn->setStyleSheet("QPushButton{ border-image: url(:/login/min_normal); }"
                                "QPushButton:hover:!pressed{ border-image: url(:/login/min_hover); }"
                                "QPushButton:hover:pressed{ border-image: url(:/login/min_press); border-style:none; }");
    connect(m_menuMinBtn, &QPushButton::pressed, this, &RegisterWndPrivate::showMinimized);

    welcome = new QLabel("欢迎注册",this);
    welcome->setFont(font);
    logo = new QLabel(this);

    logo->setPixmap(QPixmap(":/login/mychat_logo.png").
                    scaled(220,80,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    usernameLabel = new QLabel("账号",this);
    usernameLabel->setFont(font2);
    passwordLabel = new QLabel("密码",this);
    passwordLabel->setFont(font2);
    nicknameLabel = new QLabel("昵称",this);
    nicknameLabel->setFont(font2);


    usernameEdit = new QLineEdit(this);
    connect(usernameEdit,&QLineEdit::textChanged,[&](){
        m_notifyMsg->hide();
        this->setFixedSize(430,330);
    });

    passwordEdit = new QLineEdit(this);
    connect(passwordEdit,&QLineEdit::textChanged,[&](){
        m_notifyMsg->hide();
        this->setFixedSize(430,330);
    });

    nicknameEdit = new QLineEdit(this);
    connect(nicknameEdit,&QLineEdit::textChanged,[&](){
        m_notifyMsg->hide();
        this->setFixedSize(430,330);
    });

    okBtn = new QPushButton("注册",this);
    okBtn->setStyleSheet("QPushButton{border:1px solid #86949e;background-color:#f4f4f4;border-radius:3px}"
                         "QPushButton:hover{background-color:#bee7fd;}"
                         "QPushButton:pressed{background-color:#f4f4f4}");
    connect(okBtn,SIGNAL(clicked(bool)),this,SLOT(sltBtnClicked()));

    cancelBtn = new QPushButton("取消",this);
    cancelBtn->setStyleSheet("QPushButton{border:1px solid #86949e;background-color:#f4f4f4;border-radius:3px}"
                             "QPushButton:hover{background-color:#bee7fd;}"
                             "QPushButton:pressed{background-color:#f4f4f4}");
    connect(cancelBtn,SIGNAL(clicked(bool)),this,SLOT(closeWnd()));
    idLabel = new QLabel(this);
    idLabel->setFont(font2);
    idLabel->setGeometry(80,250,60,30);
    idLabel->setVisible(false);

    welcome->setGeometry(80,60,140,30);
    logo->setGeometry(180,35,210,80);

    usernameLabel->setGeometry(90,140,60,30);
    usernameEdit->setGeometry(160,143,180,25);
    usernameEdit->setStyleSheet("background:transparent;border-width:1px;border-style:outset");

    passwordLabel->setGeometry(90,190,60,30);
    passwordEdit->setGeometry(160,193,180,25);
    passwordEdit->setStyleSheet("background:transparent;border-width:1px;border-style:outset");

    nicknameLabel->setGeometry(90,240,60,30);
    nicknameEdit->setGeometry(160,243,180,25);
    nicknameEdit->setStyleSheet("background:transparent;border-width:1px;border-style:outset");

    okBtn->setGeometry(180,270,100,30);
    cancelBtn->setGeometry(300,270,100,30);
}

void RegisterWndPrivate::sltBtnClicked()
{
    qDebug() << "开始注册";
    if(usernameEdit->text().isEmpty()){
        this->setFixedSize(430,350);
        m_notifyMsg->move(0,330);
        m_notifyMsg->setText("  请输入账号");
        m_notifyMsg->show();
        return;
    }else{
        if(usernameEdit->text().size() > 20){
            this->setFixedSize(430,350);
            m_notifyMsg->move(0,330);
            m_notifyMsg->setText("  账号过长!请输入少于20个字符的账号");
            m_notifyMsg->show();
            return;
        }else{
            if(passwordEdit->text().isEmpty()){
                this->setFixedSize(430,350);
                m_notifyMsg->move(0,330);
                m_notifyMsg->setText("  请输入密码");
                m_notifyMsg->show();
                return;
            }else{
                if(passwordEdit->text().size() > 20){
                    this->setFixedSize(430,350);
                    m_notifyMsg->move(0,330);
                    m_notifyMsg->setText("  密码过长!请输入少于20个字符的密码");
                    m_notifyMsg->show();
                    return;
                }else{
                    QString text = passwordEdit->text();
                    int len = text.size();
                    for(int i = 0;i < len;i++){
                        if( (text[i] >= 'A' && text[i] <= 'Z') ||
                                (text[i] >= 'a' && text[i] <= 'z') ||
                                (text[i] >= '0' && text[i] <= '9') ||
                                text[i] == '_' || text[i] == '.'){//合法字符
                            continue;
                        }else{
                            this->setFixedSize(430,350);
                            m_notifyMsg->move(0,330);
                            m_notifyMsg->setText("  您的密码中包含非法字符!只能包含英文字母，小数点，数字，下划线");
                            m_notifyMsg->show();
                            return;
                        }
                    }

                    m_notifyMsg->hide();
                    this->setFixedSize(430,330);

                    //向服务器发送注册消息
                    nlohmann::json json;
                    json["type"]     = "register";
                    //暂时记忆
                    username = usernameEdit->text();
                    password = passwordEdit->text();

                    json["username"] = usernameEdit->text().toStdString();
                    json["password"] = passwordEdit->text().toStdString();
                    json["nickname"] = nicknameEdit->text().toStdString();

//privatewnd这个信号 被上层窗口转发给loginwidget
                    emit signalRegister(json);

                    usernameEdit->clear();
                    passwordEdit->clear();
                }
            }
        }
    }
}

void RegisterWndPrivate::closeWnd()
{
    usernameEdit->clear();
    passwordEdit->clear();

    emit closeWindow();
}


void RegisterWndPrivate::sltRegisterReply(const nlohmann::json &json)
{
    if(json.value("istrue",false)){
        m_notifyMsg->move(0,330);
        m_notifyMsg->setText("  出了点小差错，服务器注册失败");
        m_notifyMsg->show();
    }

//id为服务器上天按序分配
    int id = json.value("id",0);

    qDebug() << "注册成功!"
    << " id:" << id;

    welcome->setText("注册成功!");
    usernameEdit->setVisible(false);
    passwordEdit->setVisible(false);

    usernameLabel->setGeometry(80,120,250,30);
    usernameLabel->setText(username + ",欢迎使用.");

    idLabel->setGeometry(80,170,350,30);
    idLabel->setVisible(true);
    idLabel->setText("您的id为: " + QString::number(id));

    passwordLabel->setGeometry(80,210,250,30);
    passwordLabel->setText("密码为: " + password);

    okBtn->setText("返回登陆");
    disconnect(okBtn,&QPushButton::clicked,this,&RegisterWndPrivate::sltBtnClicked);
    connect(okBtn,&QPushButton::clicked,this,&RegisterWndPrivate::closeWnd);

    //disconnect(okBtn,SIGNAL(clicked(bool)),this,SLOT(sltBtnClicked()));
    //(okBtn,SIGNAL(clicked(bool)),this,SLOT(closeWnd()));

}


//---------------------------------


RegisterWnd::RegisterWnd(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    mainWnd = new RegisterWndPrivate;

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addWidget(mainWnd);
    pLayout->setContentsMargins(20, 20, 20, 20);

    QGraphicsDropShadowEffect *pEffect = new QGraphicsDropShadowEffect(mainWnd);
    pEffect->setOffset(0, 0);
    pEffect->setColor(QColor(QStringLiteral("black")));
    pEffect->setBlurRadius(30);
    mainWnd->setGraphicsEffect(pEffect);

    connect(mainWnd,&RegisterWndPrivate::closeWindow,this,&RegisterWnd::sltCloseWnd);
    //connect(mainWnd,SIGNAL(closeWindow()),this,SLOT(sltCloseWnd()));

    connect(mainWnd,&RegisterWndPrivate::closeWindow,this,&RegisterWnd::sltCloseWnd);
    //信号转发,否则外层loginwidget拿不到对象
    connect(mainWnd,&RegisterWndPrivate::signalRegister,this,&RegisterWnd::signalRegister);


    //connect(mainWnd,SIGNAL(signalRegister(const nlohmann::json &)),
    //        this,SIGNAL(signalRegister(const QJsonValue &)));
}

void RegisterWnd::sltCloseWnd()
{
    emit closeWindow(this->pos());
    this->close();
}

void RegisterWnd::sltRegisterOK(const nlohmann::json &data)
{
    mainWnd->sltRegisterReply(data);
}

void RegisterWnd::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        pressedPoint = event->globalPosition(); // 鼠标按下时的位置
        startPos = this->pos(); // 窗口按下时的初始位置
        isPressed = true;
        event->accept();
    }
}
void RegisterWnd::mouseReleaseEvent(QMouseEvent *event)
{
    isPressed = false;
    event->accept();
}


void RegisterWnd::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && isPressed) {
        // 计算鼠标移动的距离
        QPointF delta = event->globalPosition() - pressedPoint;
        // 计算窗口的新位置：窗口的初始位置 + 鼠标移动的距离
        QPointF newPos = startPos + delta;
        move(newPos.toPoint());
        event->accept();
    }
}
