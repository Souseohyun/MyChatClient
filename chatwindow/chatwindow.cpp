#include "chatwindow.h"

#include "chatwindow/ui_chatwindow.h"

ChatWindow::ChatWindow(boost::asio::ip::tcp::socket socket,int& user_id,QWidget *parent)
    : chatNetworkManager_(std::move(socket)),userId_(user_id),QWidget(parent)
    , ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    //设置无系统窗口
    this->setWindowFlags(Qt::SplashScreen|Qt::FramelessWindowHint);
    //setMouseTracking(true);// 设置鼠标跟踪，不然只会在鼠标按下时才会触发鼠标移动事件
    //setWindowIcon(QIcon(""));

    //初始化图像服务器
    imageNetworkManager_.ConnectToServer("172.30.229.221","23611");



    //连接网络类信号与chatwindow的槽
    //信息与信息槽
    connect(&chatNetworkManager_, &NetworkManager::messageReceived,
            this, &ChatWindow::displayReceivedData);
    //头像与更新myPic槽
    connect(&imageNetworkManager_, &NetworkManager::headerReceived,
            this, &ChatWindow::updateReceivedHeader);
    //图像与图像槽
    connect(&imageNetworkManager_, &NetworkManager::imageReceived,
            this, &ChatWindow::updateReceivedImage);

    // addMessage("Hello!", "12:00", QNChatMessage::User_Me);


    chatNetworkManager_.ListeningFromChatSrv();

    //如果此时并没有建立好链接(丑陋的等待，但无伤大雅，顶多1s，你也可以优化成条件变量互斥量）
    while(!imageNetworkManager_.isConnect_){
        std::cout<<"image connect ing"<<std::endl;
    }
    std::string http = imageNetworkManager_.as_HttpGetImageByUserId(userId_);
    imageNetworkManager_.SendToImageServer(http);
    imageNetworkManager_.RecvMyheadImageSrv();

    while(!imageNetworkManager_.isConnect_){
        std::cout<<"RecvMyheadImageSrv ing"<<std::endl;
    }
    imageNetworkManager_.ListeningFromImageSrv();


    ui->listWidget->update(); // 更新列表

    //焦点设置于pushText
    ui->textEdit->setFocus();

    // 为输入框安装事件过滤器
    ui->textEdit->installEventFilter(this);

}

ChatWindow::~ChatWindow()
{
    chatNetworkManager_.CloseSocket();
    delete ui;
}

void ChatWindow::on_toolButton_clicked()
{

    this->close();
}

void ChatWindow::mousePressEvent(QMouseEvent *event)
{
    isPressedWidget = true; // 当前鼠标按下的即是QWidget而非界面上布局的其它控件
    last = event->globalPosition().toPoint(); // 使用 toPoint() 来转换 QPointF 到 QPoint
}

void ChatWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (isPressedWidget)
    {
        QPoint delta = event->globalPosition().toPoint() - last;
        last = event->globalPosition().toPoint();
        move(x() + delta.x(), y() + delta.y());
    }
}

void ChatWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint delta = event->globalPosition().toPoint() - last;
    move(x() + delta.x(), y() + delta.y());
    isPressedWidget = false; // 鼠标松开时，置为false
}

void ChatWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event); // 调用基类实现
    // 重新计算布局和尺寸
    //updateMessageLayout(event->size());
}


void ChatWindow::addMessage(const QString &text, const QString &time, QNChatMessage::User_Type userType,int addUserId) {
    // 创建一个新的 QNChatMessage
    QNChatMessage *message = new QNChatMessage();

    std::string http = imageNetworkManager_.as_HttpGetImageByUserId(addUserId);
    imageNetworkManager_.SendToImageServer(http);
    //发送该报文后，imageNetwork中的监听函数会即使emit信号更新youPic
    if(userType == QNChatMessage::User_Type::User_Me){
        message->SetHeaderImage(userType,myPic);
    }else if(userType == QNChatMessage::User_Type::User_She){
        message->SetHeaderImage(userType,youPic);
    }else{
        qDebug()<<"SetHeaderImage Error";
    }


    message->setText(text, time, QSize(), userType); // 设置文本，时间和用户类型

    // 使用 fontRect 函数获取 QNChatMessage 控件的尺寸
    QSize messageSize = message->fontRect(text);

    // 创建一个 QListWidgetItem
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(messageSize ); // 设置大小提示为 QNChatMessage 的推荐大小

    // 将 QNChatMessage 添加到 QListWidget 中
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, message);
}


//将服务器读来内容借助addMessage同步到显示listwidget中
void ChatWindow::displayReceivedData(const QString& data,int userId) {

    addMessage(data, "12:00", QNChatMessage::User_She,userId);

    //客户端从服务器读来文本信息的成功回调函数中
    //emit发来的信息最好带user_id，
    //先检索，如果存储地址已经有该user_id的头像了，那就不触发向服务器请求图像的逻辑，直接从本地应用
    //如果本地没有，拼接写死的存储路径和user_id当作图片文件名做到将图片存储到本地
}

//本槽函数，仅更新自身成员QPixmap myPic
void ChatWindow::updateReceivedHeader(const QByteArray &headData)
{
    QPixmap pixmap;
    if (pixmap.loadFromData(headData)) {
        myPic = pixmap;

    }
}

//用于更新其他人的消息气泡头像
void ChatWindow::updateReceivedImage(const QByteArray &imageData)
{
    QPixmap pixmap;
    if (pixmap.loadFromData(imageData)) {
        youPic = pixmap;
        // 可以在这里更新 UI 或进行其他操作
    }

}


//事件过滤器处理函数(针对textEdit的回车事件）
bool ChatWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->textEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            onTextEidtReturnPressed();
            return true; // 事件已处理
        }
    }
    return QWidget::eventFilter(obj, event);
}


//当按下回车键
void ChatWindow::onTextEidtReturnPressed(){
    QString text = ui->textEdit->toPlainText().trimmed();
    if (!text.isEmpty()) {
        // 同步显示到 listWidget 控件
        addMessage(text,"12:00",QNChatMessage::User_Me,this->userId_);

        // 使用 nlohmann::json 创建 JSON 对象
        nlohmann::json json;
        json["type"] = "message_text";
        json["userid"] = this->userId_;
        json["text"] = text.toStdString();

        chatNetworkManager_.SendToServer(json);
        // 清空发送框 textEdit 控件内容
        ui->textEdit->clear();
    }
}
