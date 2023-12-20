#include "loginwidget.h"
#include "chatwindow/chatwindow.h"

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

    ui->hideButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    ui->closeButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));

    this->CreatBackgroud();
    this->CreatHeadPic();
    this->CreatLogo();
    //连接服务器
    networkManager_.ConnectToServer("172.30.229.221","23610");

    //绑定信号与槽
    //connect(&networkManager_, &NetworkManager::dataReceived, this, &MainWindow::displayReceivedData);
    connect(&networkManager_, &NetworkManager::loginResponseReceived,this, &LoginWidget::onLoginResponseReceived);
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

    // 通过 NetworkManager 发送 JSON 数据到服务器
    networkManager_.SendToServer(json);

    //该函数用于接收服务器发回的确认信息，并emit本窗口调用onLogin槽函数
    networkManager_.ReceiveServerResponse();
}


//networkManager登录核验成功后发射信号调用该槽函数,int不能传&，信号要求可存储与复制
void LoginWidget::onLoginResponseReceived(bool success, const QString& message,int user_id) {
    std::cout<<"into onLoginResponseReceived"<<std::endl;
    if (success) {
        // 登录成功，创建并显示新的业务窗口
        // auto businessWindow = new BusinessWidget(); // 假设 BusinessWindow 是业务窗口的类
        // businessWindow->show();

        auto chatwindow = new ChatWindow(std::move(networkManager_.GetSocket()),user_id);
        QIcon icon(":/chatApp.ico");

        //debug_用


        chatwindow->setWindowTitle("Mychat");
        chatwindow->setWindowIcon(icon);

        chatwindow->show();

        // 关闭登录窗口
        this->close();
    } else {
        // 登录失败，显示错误信息
        QMessageBox::warning(this, "Login Failed", message);
    }
}
