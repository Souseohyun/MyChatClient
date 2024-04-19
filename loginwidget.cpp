#include "loginwidget.h"
#include "chatwindow/chatwindow.h"
#include "clientwindow.h"
#include "sqlite/sqldatabase.h"

#include "chatwindow/bubbleinfo.h"

#include "ui_loginwidget.h"

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    //设置透明背景
    this->setAttribute(Qt::WA_TranslucentBackground);
    //设置无系统边框
    this->setWindowFlags(Qt::SplashScreen|Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint);


    this->CreatShadow();        //创建无边框窗口阴影

    ui->setupUi(this);

    MsgNotify= new QLabel(this);
    MsgNotify->setFixedSize(430, 20);
    MsgNotify->setStyleSheet("background-color:#09a3dc;font-size: 12px;font-family:Microsoft YaHei;border-radius:10px");
    MsgNotify->hide();

    registerAccount = new ClickLabel(this);
    registerAccount->setText(tr("注册账号"));
    registerAccount->setStyleSheet("font-size: 12px;font-family:Microsoft YaHei; color: #a6a6a6;");
    registerAccount->setFixedSize(55, 20);
    registerAccount->move(12, 300);
    connect(registerAccount, &ClickLabel::clickSignal, this, &LoginWidget::openRegisterWnd);


    registerWnd = new RegisterWnd();
    registerWnd->hide();
    //registerWnd转发内部privateWnd的信号被该slt接收
    connect(registerWnd,&RegisterWnd::signalRegister,this,&LoginWidget::sltRegisterGo);

    //net传来的注册回应
    connect(&networkManager_,&NetworkManager::RegisterRelpyRece,registerWnd,&RegisterWnd::sltRegisterOK);

    //register关闭
    connect(registerWnd,&RegisterWnd::closeWindow,this,&LoginWidget::sltRegisterClose);

    ui->hideButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    ui->closeButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));

    this->CreatBackgroud();
    this->CreatHeadPic();
    this->CreatLogo();
    //连接服务器
#ifdef _RELEASE
    networkManager_.ConnectToServer("122.51.38.77","23610");
#else
    networkManager_.ConnectToServer("172.30.229.221","23610");
#endif

    //绑定信号与槽
    //connect(&networkManager_, &NetworkManager::dataReceived, this, &MainWindow::displayReceivedData);
    connect(&networkManager_, &NetworkManager::loginResponseReceived,this, &LoginWidget::onLoginResponseReceived);
    connect(&networkManager_, &NetworkManager::loginResponseReceivedWithFriends,
            this, &LoginWidget::onLoginResponseReceivedWithFriends);

    connect(&networkManager_,&NetworkManager::msgResponseReceived,this,&LoginWidget::onMsgResponseReceived);
}

LoginWidget::~LoginWidget()
{
    networkManager_.CloseSocket();
    delete ui;
}

void LoginWidget::on_hideButton_clicked()
{
    this->hide();
}


void LoginWidget::on_closeButton_clicked()
{
    this->close();
}

void LoginWidget::mousePressEvent(QMouseEvent *event)
{
    isPressedWidget = true; // 当前鼠标按下的即是QWidget而非界面上布局的其它控件
    last = event->globalPosition().toPoint(); // 使用 toPoint() 来转换 QPointF 到 QPoint
}

void LoginWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isPressedWidget)
    {
        QPoint delta = event->globalPosition().toPoint() - last;
        last = event->globalPosition().toPoint();
        move(x() + delta.x(), y() + delta.y());
    }
}

void LoginWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint delta = event->globalPosition().toPoint() - last;
    move(x() + delta.x(), y() + delta.y());
    isPressedWidget = false; // 鼠标松开时，置为false
}

void LoginWidget::CreatHeadPic()
{
    // 加载原始图片
    QPixmap originalPixmap(":/Sulli.jpg");
    QPixmap x(":/Icons/MainWindow/close.png");
    // 确保图片已经加载
    if(originalPixmap.isNull()) {
        qDebug() << "图片加载失败";
        return;
    }

    // 裁剪图片为正方形
    int side = qMin(originalPixmap.width(), originalPixmap.height());
    QPixmap squarePixmap = originalPixmap.copy((originalPixmap.width() - side) / 2,
                                               (originalPixmap.height() - side) / 2, side, side);

    // 创建一个相同大小的透明Pixmap
    QPixmap circlePixmap(side, side);
    circlePixmap.fill(Qt::transparent);

    // 使用QPainter进行圆形裁剪
    QPainter painter(&circlePixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(squarePixmap));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, side, side);

    // 将裁剪后的圆形Pixmap设置到QLabel上
    ui->headPicLabel->setPixmap(circlePixmap);

    // 计算圆形边界的样式表
    QString styleSheet = QString(
        "QLabel {"
        "  border-radius: %1px;" // 半径设置为label尺寸的一半
        "  background-color: transparent;" // 背景色设置为透明
        "}"
        );
    ui->headPicLabel->setStyleSheet(styleSheet.arg(side / 2));

    // 设置图片尺寸自适应Label

    ui->headPicLabel->setScaledContents(true);
}

void LoginWidget::CreatBackgroud()
{
    QMovie* movie = new QMovie(":/login/login_backgroud.gif");
    ui->backLabel->setAlignment(Qt::AlignCenter);    // 设置标签对齐方式为居中
    movie->setScaledSize(ui->backLabel->size()+QSize(50,50));  // 将 GIF 缩放到略大 QLabel 的大小

    ui->backLabel->setMovie(movie);
    ui->backLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 设置大小策略
    movie->start();
}

void LoginWidget::CreatLogo()
{
    QPixmap logoPixmap(":/login/mychat_logo.png");
    // 假设我们想要将 logo 放大到原来的 1.5 倍大小
    QSize newSize = logoPixmap.size() * 1.5;
    QPixmap scaledLogo = logoPixmap.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 放大图像并保持宽高比

    ui->logoLabel->setPixmap(scaledLogo);  // 将缩放后的 QPixmap 设置到 QLabel
    ui->logoLabel->setAlignment(Qt::AlignCenter);  // 如果需要，可以设置标签的对齐方式为居中

}

void LoginWidget::CreatShadow()
{

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor("#444444"));
    shadow->setBlurRadius(30);
    this->setGraphicsEffect(shadow);
    this->setContentsMargins(1,1,1,1);


}

void LoginWidget::ShowNotify(QString msg)
{
    this->setFixedSize(430, 350);
    MsgNotify->move(0, 330);
    MsgNotify->setText("  " + msg);
    MsgNotify->show();
}

void LoginWidget::HideNotify()
{
    this->setFixedSize(430, 330);
    MsgNotify->hide();
}








//点击登录按钮后，用户名userEdit，密码passEdit
void LoginWidget::on_pushButton_clicked()
{
    // 获取用户名和密码
    QString username = ui->userEdit->text();
    QString password = ui->passEdit->text();

    // 使用 nlohmann::json 创建 JSON 对象
    nlohmann::json json;
    json["type"] = "login";
    json["username"] = username.toStdString();
    json["password"] = password.toStdString();  // 注意：明文传输密码是不安全的

    MyConfig::strUserName = username;
    MyConfig::strPassWord = password;

    // 通过 NetworkManager 发送 JSON 数据到服务器
    networkManager_.SendToServer(json);

    //该函数用于接收服务器发回的确认信息，并emit本窗口调用onLogin槽函数
    networkManager_.ReceiveServerResponse();
}


//networkManager登录核验成功后发射信号调用该槽函数,int不能传&，信号要求可存储与复制
void LoginWidget::onLoginResponseReceived(bool success, const QString& message,int user_id) {
    std::cout<<"into nofriend recv"<<std::endl;
    if (success) {
        //身份核验通过后，创建属于该用户的本地数据库
        SqlDataBase::Instance()->openDb(user_id);
        MyConfig::userId = user_id;
        MyConfig::SaveUserInfo();

        ClientWindow* cli = new ClientWindow(std::move(networkManager_.GetSocket()), user_id);

        cli->show();


        this->close();
    } else {
        // 登录失败，显示错误信息
        ShowNotify("登录失败，账号或密码错误！");
        //QMessageBox::warning(this, "Login Failed", message);
    }
}


/*
    if (responseJson.contains("friends") && responseJson["friends"].is_array()) {
        auto friendsJson = responseJson["friends"];

        for (const auto& friendJson : friendsJson) {
            std::cout << "Friend ID: " << friendJson.value("friend_id", 0) << std::endl;
            if (friendJson.contains("markname") && !friendJson["markname"].is_null()) {
                std::cout << "Markname: " << friendJson["markname"].get<std::string>() << std::endl;
            } else {
                std::cout << "Markname is missing or null" << std::endl;
            }
            if (friendJson.contains("teamname") && !friendJson["teamname"].is_null()) {
                std::cout << "Teamname: " << friendJson["teamname"].get<std::string>() << std::endl;
            } else {
                std::cout << "Teamname is missing or null" << std::endl;
            }
        }
    }
*/
void LoginWidget::onLoginResponseReceivedWithFriends(bool success, const QString& message,
                                                     int user_id, const nlohmann::json& friends) {
    std::cout << "into ReceivedWithFriends" << std::endl;
    if (success) {
        // 身份核验通过后，创建属于该用户的本地数据库
        SqlDataBase::Instance()->openDb(user_id);
        qDebug() << "Processing friends data...";

        MyConfig::userId = user_id;
        MyConfig::SaveUserInfo();

        // 遍历并打印好友信息
        for (const auto& friendJson : friends) {
            qDebug() << "Friend JSON:" << QString::fromStdString(friendJson.dump());

            try {
                int friendID = friendJson["friend_id"].get<int>();
                //暂用markname顶替name
                QString name = QString::fromStdString(friendJson["markname"]);
                QString teamname = QString::fromStdString(friendJson["teamname"]);
                QString markname = QString::fromStdString(friendJson["markname"]);

                qDebug() << "Friend ID:" << friendID << ", Name:" << name << ", Teamname:" << teamname << ", Markname:" << markname;

                // 将好友信息添加到数据库
                SqlDataBase::Instance()->addFriend(user_id, friendID, name, teamname, markname);
            } catch (const std::exception& e) {
                qDebug() << "Error parsing friend JSON:" << e.what();
            }


        }
        // 创建并显示客户端窗口
        ClientWindow* cli = new ClientWindow(std::move(networkManager_.GetSocket()), user_id);
        cli->show();
        qDebug()<<"我现在new了client";
        //QApplication::quit();

        this->close();
        //QApplication::quit();
    }
     else {
        // 显示错误信息
        std::cerr << "Login failed: " << message.toStdString() << std::endl;
    }
}

void LoginWidget::onMsgResponseReceived(const int user_id, const nlohmann::json &msgJson)
{
    qDebug()<<"__DEBUG:INTO MsgResponse";
    SqlDataBase::Instance()->openDb(user_id);
    // 遍历每条消息
    for (const auto& msg : msgJson) {
        // 创建一个新的BubbleInfo对象来存储消息信息
        BubbleInfo *info = new BubbleInfo;

        info->srcid = msg.value("src_id",0);

        info->myid = user_id;
        info->destid = msg.value("dest_id",0);
        info->isGroup = false;      //以后可能添加功能，尚不能如此武断
        info->message = QString::fromStdString(msg.value("text",""));
        //info->time = msg.value("datetime","")     // 消息时间
        // info->msgType, info->fileSize, info->downloaded等其他字段根据需要设置

        // 使用数据库对象添加消息历史
        SqlDataBase::Instance()->addHistoryMsg(info);

        // 清理创建的info对象以避免内存泄漏
        delete info;
    }
}

void LoginWidget::openRegisterWnd()
{

    qDebug()<<"into register";
    registerWnd->move(this->pos().x(),this->pos().y()/* + 80*/);
    registerWnd->show();
    this->hide();

}

void LoginWidget::sltRegisterClose(QPoint pos)
{
    this->move(pos);
    this->show();
}

void LoginWidget::sltRegisterGo(const nlohmann::json &js)
{
    qDebug()<<"准备发送注册信息";
    //this->show();
    networkManager_.SendToServer(js);
    networkManager_.ListeningFromChatSrv();
}





