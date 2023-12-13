#include "chatwindow.h"

#include "chatwindow/ui_chatwindow.h"

ChatWindow::ChatWindow(boost::asio::ip::tcp::socket socket,std::string user_id,QWidget *parent)
    : networkManager_(std::move(socket)),QWidget(parent)
    , ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    //设置无系统窗口
    this->setWindowFlags(Qt::SplashScreen|Qt::FramelessWindowHint);
    //setMouseTracking(true);// 设置鼠标跟踪，不然只会在鼠标按下时才会触发鼠标移动事件
    //setWindowIcon(QIcon(""));


    //连接网络类信号与chatwindow的槽
    connect(&networkManager_, &NetworkManager::messageReceived,
            this, &ChatWindow::displayReceivedData);

    //connect(&networkManager_, &NetworkManager::dataReceived, this, &ChatWindow::displayReceivedData);=-

    // addMessage("Hello!", "12:00", QNChatMessage::User_Me);
    // addMessage("Hi, how are you?", "12:01", QNChatMessage::User_She);
    // addMessage("Fuck u Windows;Fuck u Qt", "12:01", QNChatMessage::User_Me);

    networkManager_.ListeningFromSrv();


    ui->listWidget->update(); // 更新列表

    //焦点设置于pushText
    ui->textEdit->setFocus();

    // 为输入框安装事件过滤器
    ui->textEdit->installEventFilter(this);

}

ChatWindow::~ChatWindow()
{
    networkManager_.CloseSocket();
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


void ChatWindow::addMessage(const QString &text, const QString &time, QNChatMessage::User_Type userType) {
    // 创建一个新的 QNChatMessage
    QNChatMessage *message = new QNChatMessage();
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
void ChatWindow::displayReceivedData(const QString& data) {

    addMessage(data, "12:00", QNChatMessage::User_She);
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

void ChatWindow::onTextEidtReturnPressed(){
    QString text = ui->textEdit->toPlainText().trimmed();
    if (!text.isEmpty()) {
        // 同步显示到 listWidget 控件
        addMessage(text,"12:00",QNChatMessage::User_Me);

        // 使用 nlohmann::json 创建 JSON 对象
        nlohmann::json json;
        json["type"] = "message_text";
        json["text"] = text.toStdString();

        networkManager_.SendToServer(json);
        // 清空发送框 textEdit 控件内容
        ui->textEdit->clear();
    }
}
