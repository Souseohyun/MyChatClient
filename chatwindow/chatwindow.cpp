#include "chatwindow.h"

#include "chatwindow/ui_chatwindow.h"
#include "base/emojipicker.h"

#include <QWidgetAction>

ChatWindow::ChatWindow(NetworkManager& chatNet,NetworkManager& imageNet, int& user_id,int& son_id,QString name,QPixmap myHead,QPixmap youHead, QWidget *parent)
    : QWidget(parent),
    chatNetworkManager_(chatNet), imageNetworkManager_(imageNet),
    userId_(user_id),toId_(son_id),
    myPic(myHead),youPic(youHead),
    ui(new Ui::ChatWindow) {

    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

    //图像与图像槽
    connect(&imageNetworkManager_, &NetworkManager::imageReceived,
            this, &ChatWindow::updateReceivedImage);

    //addMessage("Hello!", "12:00", QNChatMessage::User_Me,userId_);

    //设置聊天对方用户名
    chatName = new QLabel(this);
    chatName->setText(name); // 这里设置为 "text"，您可以稍后改为正确的用户名

    ui->pushButton->setText("发 送");
    ui->pushButton->setStyleSheet("QPushButton {"
                                  "    background-color: rgb(233, 233, 233);"
                                  "    border: none;"
                                  "}"
                                  "QPushButton:hover {"
                                  "    background-color: rgb(210, 210, 210);"
                                  "}"
                                  "QPushButton:pressed {"
                                  "    background-color: rgb(198, 198, 198);"
                                  "}");

    setUIStyle();

    // 更新列表
    ui->listWidget->update();

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

    //this->close();
    QApplication::quit();
}


void ChatWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event); // 调用基类实现
    // 重新计算布局和尺寸
    //updateMessageLayout(event->size());
}


void ChatWindow::addMessage(const QString &text, const QString &time, QNChatMessage::User_Type userType,int addUserId) {
    // 创建一个新的 QNChatMessage
    QNChatMessage *message = new QNChatMessage(ui->listWidget);
    message->setFixedWidth(ui->listWidget->width() -  ui->listWidget->contentsMargins().left()); // 为滚动条留出空间
    // 该代码保证了qnchatmessage自身的wdith大小
    if(userType == QNChatMessage::User_Type::User_Me){
        message->SetHeaderImage(userType,myPic);
    }else if(userType == QNChatMessage::User_Type::User_She){
        message->SetHeaderImage(userType,youPic);
    }else{
        qDebug()<<"SetHeaderImage Error";
    }

    message->setText(text, time, QSize(ui->listWidget->width(), 0), userType); // 设置文本，时间和用户类型
    // 获取 QNChatMessage 控件的尺寸
    QSize messageSize = message->fontRect(text);

    // 创建一个 QListWidgetItem
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    // 设置大小提示为 QNChatMessage 的推荐大小，并调整以适应 QListWidget 的宽度
    item->setSizeHint(messageSize);

    // 将 QNChatMessage 添加到 QListWidget 中
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, message);

    // 确保 QListWidget 滚动到新消息
    ui->listWidget->scrollToBottom();

    //与左侧会话栏cellviewson的lastMessage交互更新
    if(addUserId == userId_){
        emit messageAdded(toId_, text);
    }else{
        emit messageAdded(addUserId, text);
    }

}

void ChatWindow::addX()
{
    // 初始化一个简单的 HTML 字符串
    const QString htmlStr = "<html><body><h1>Hello, World!</h1><p>This is a <b>test</b> HTML string.</p></body></html>";

    // 创建一个 QLabel 来显示 HTML
    QLabel *label = new QLabel(this);
    label->setText(htmlStr);
    label->setTextFormat(Qt::RichText); // 确保 QLabel 以富文本格式显示内容
    label->setWordWrap(true); // 如果需要，启用自动换行

    // 设置 QLabel 的一些其他属性，例如大小和位置
    label->setGeometry(10, 10, 300, 100); // 根据需要调整大小和位置

    // 显示 QLabel
    label->show();


}




#if 0
void ChatWindow::addMessage(const QString htmlContent, const QString time, QNChatMessage::User_Type userType, int addUserId) {
    // 创建一个新的 QNChatMessage
    QNChatMessage *message = new QNChatMessage(ui->listWidget);
    message->setFixedWidth(ui->listWidget->width() - ui->listWidget->contentsMargins().left()); // 为滚动条留出空间

    // 设置头像
    if(userType == QNChatMessage::User_Type::User_Me){
        message->SetHeaderImage(userType, myPic);
    } else if(userType == QNChatMessage::User_Type::User_She){
        message->SetHeaderImage(userType, youPic);
    } else {
        qDebug() << "SetHeaderImage Error";
    }

    // 使用新的 setHtml 方法
    QSize qs =  QSize(ui->listWidget->width(), 0);
    message->setHtml(htmlContent, time,qs, userType); // 设置 HTML 内容和用户类型

    // 使用 QTextDocument 来计算 HTML 内容的尺寸
    QTextDocument doc;
    doc.setHtml(htmlContent);
    doc.setTextWidth(ui->listWidget->width()); // 使用 QListWidget 的宽度作为文档的宽度
    //QSize docSize(doc.idealWidth(), doc.size().height()); // 计算文档理想宽度和高度
    QSize docSize = calculateDocumentSize(htmlContent);
    qDebug()<<"HTML:"<<htmlContent;
    qDebug()<<"NOW DOC SIZE:"<<docSize;
    // 创建一个 QListWidgetItem 并设置大小提示
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(docSize); // 使用文档大小设置大小提示

    // 将 QNChatMessage 添加到 QListWidget 中
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, message);

    // 确保 QListWidget 滚动到新消息
    ui->listWidget->scrollToBottom();

    // 更新左侧会话栏的最后一条消息
    if(addUserId == userId_){
        emit messageAdded(toId_, htmlContent);
    } else {
        emit messageAdded(addUserId, htmlContent);
    }
}

#endif



QSize  ChatWindow::calculateDocumentSize(const QString &htmlContent)
{
    QTextDocument doc;
    doc.setHtml(htmlContent);
    doc.setDefaultFont(ui->listWidget->font()); // 尝试设置一个较大的默认字体

    // 尝试固定宽度以测试尺寸计算
    int fixedWidth = 300; // 临时宽度，用于测试
    doc.setTextWidth(fixedWidth);

    // 确保文档更新布局
    QCoreApplication::processEvents();

    // 计算理想尺寸
    QSize docSize(doc.idealWidth(), static_cast<int>(doc.size().height()));

    qDebug() << "Calculated doc size:" << docSize;
    return docSize; // 返回文档的尺寸
}


/*废弃*/
//本槽函数，仅更新自身成员QPixmap myPic
void ChatWindow::updateReceivedHeader(const QByteArray &headData)
{
    //自身头像交付给clientwindow传入
    // QPixmap pixmap;
    // if (pixmap.loadFromData(headData)) {
    //     myPic = pixmap;

    // }
}

/*废弃*/
//用于更新其他人的消息气泡头像
void ChatWindow::updateReceivedImage(const QByteArray &imageData)
{
    QPixmap pixmap;
    if (pixmap.loadFromData(imageData)) {
        youPic = pixmap;
        // 可以在这里更新 UI 或进行其他操作
        imageNetworkManager_.readyToSend_ = true;

    }

}


//事件过滤器处理函数(针对textEdit的回车事件）
bool ChatWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->textEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            qDebug()<<"我在event里";
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
        //qDebug()<<"add之前";
        addMessage(text,"12:00",QNChatMessage::User_Me,this->userId_);
        //qDebug()<<"add之后";
        // 使用 nlohmann::json 创建 JSON 对象
        nlohmann::json json;
        json["type"] = "message_text";
        json["srcid"] = this->userId_;
        json["destid"]= this->toId_;
        json["text"] = text.toStdString();

        qDebug()<<"Send之前";
        chatNetworkManager_.SendToServer(json);

        //此时json中text用cout出来会乱码，发给服务器是能正常读取
        //qDebug()<<"客户端发给服务器的信息：";
        std::cout<<json<<std::endl;
        // 清空发送框 textEdit 控件内容
        ui->textEdit->clear();
    }
}

#if 0
void ChatWindow::onTextEidtReturnPressed(){
    QString htmlContent = ui->textEdit->toHtml(); // 获取包含文本和图像的 HTML 内容
    if (!htmlContent.isEmpty()) {
        // 同步显示到 listWidget 控件
        addMessage(htmlContent, "12:00", QNChatMessage::User_Me, this->userId_);

        // 创建 JSON 对象
        nlohmann::json json;
        json["type"] = "message_html";
        json["srcid"] = this->userId_;
        json["destid"] = this->toId_;
        json["text"] = htmlContent.toStdString(); // 发送 HTML 内容

        // 发送 JSON 到服务器
        qDebug()<<"Send之前";
        chatNetworkManager_.SendToServer(json);

        // 清空发送框 textEdit 控件内容
        ui->textEdit->clear();
    }
}
#endif
void ChatWindow::setUIStyle(){
    // 设置 listWidget 和 textEdit 的样式，使其拥有细微的灰色边缘线
    QString subtleBorderStyle = "1px solid #dcdcdc"; // 可以调整颜色和宽度以更好地匹配您的UI风格

    ui->listWidget->setStyleSheet(QString("QListWidget {"
                                          "border: %1;"
                                          "border-radius: 4px;" // 如果您想要圆角可以调整此值
                                          "}").arg(subtleBorderStyle));

    ui->textEdit->setStyleSheet(QString("QTextEdit {"
                                        "border: %1;"
                                        "border-radius: 4px;"
                                        "}").arg(subtleBorderStyle));


    chatName->setStyleSheet("QLabel {"
                            "margin-left: 10px;" // 根据需要调整间距
                            "color: black;" // 可以调整颜色
                            "font-size: 26px;" // 字体大小
                            "}");

    // 设置 chatName(QLabel) 的位置和大小
    chatName->setGeometry(ui->listWidget->geometry().x() - 10, ui->listWidget->geometry().y() - 40, ui->listWidget->width(), 35); // 根据需要调整高度和间距


    //设置emoji，pic，file，screen的样式
    QPixmap emojiPic(":/Icons/MainWindow/emoji.png");



    QStringList tmp;
    tmp << ":/Icons/MainWindow/emoji.png"
        << ":/Icons/MainWindow/emoji2.png"
        << ":/Icons/MainWindow/emoji.png";
    emoji = new MyButton(this,tmp,QSize(25,25),NormalBtn);

    emojiMenu = new QMenu(this);
    EmojiPicker *picker = new EmojiPicker(this);
    QWidgetAction *action = new QWidgetAction(emojiMenu);
    action->setDefaultWidget(picker);
    emojiMenu->addAction(action);

    //某表情被选中后，添加到textEdit中
    connect(picker, &EmojiPicker::emojiSelected, this, [this](const QString &imagePath){
        // 创建 HTML 格式的图像标签，并插入到 QTextEdit
        QString imgTag = QString("<img src='%1' />").arg(imagePath);
        ui->textEdit->insertHtml(imgTag);

        // 确保在插入图片后，textEdit 能继续输入文本
        ui->textEdit->moveCursor(QTextCursor::End);
    });

    //emoji->setMenu(emojiMenu);  不采取直接setMenu的方式（会出现下拉框不美观且不够准确）
    //连接emoji按钮点击信号与用于显示菜单的槽函数
    connect(emoji, &QPushButton::clicked, this, [this]() {
        //emojiMenu->exec(emoji->mapToGlobal(QPoint(0, emoji->height())));
        // 获取按钮的位置
        QPoint menuPos = emoji->mapToGlobal(QPoint(0, 0));
        // 计算菜单的Y坐标，使其出现在按钮上方
        int menuY = menuPos.y() - emojiMenu->sizeHint().height();
        // 显示菜单
        emojiMenu->exec(QPoint(menuPos.x(), menuY));
    });

    tmp.clear();
    tmp << ":/Icons/MainWindow/picture.png"
        << ":/Icons/MainWindow/picture2.png"
        << ":/Icons/MainWindow/picture.png";
    picture = new MyButton(this,tmp,QSize(25,25),NormalBtn);

    tmp.clear();
    tmp << ":/Icons/MainWindow/file.png"
        << ":/Icons/MainWindow/file2.png"
        << ":/Icons/MainWindow/file.png";
    file = new MyButton(this,tmp,QSize(25,25),NormalBtn);

    tmp.clear();
    tmp << ":/Icons/MainWindow/screenshot.png"
        << ":/Icons/MainWindow/screenshot2.png"
        << ":/Icons/MainWindow/screenshot.png";
    screenshot = new MyButton(this,tmp,QSize(25,25),NormalBtn);

    // 由于textEdit和listWidget历史原因是ui设计，所以不方便在使用布局
    // 转而采用像素位置
    int x = 5; // 按钮的起始X坐标
    int y = ui->listWidget->geometry().bottom() + 10; // listWidget的底部往下10个像素
    int buttonWidth = 25; // 按钮宽度
    int buttonHeight = 25; // 按钮高度
    int padding = 5; // 按钮之间的间距

    qDebug()<<"x = 10, y = "<<y;

    emoji->setGeometry(x, y, buttonWidth, buttonHeight);
    x += buttonWidth + padding; // 更新x坐标，为下一个按钮留出空间

    picture->setGeometry(x, y, buttonWidth, buttonHeight);
    x += buttonWidth + padding;

    file->setGeometry(x, y, buttonWidth, buttonHeight);
    x += buttonWidth + padding;

    screenshot->setGeometry(x, y, buttonWidth, buttonHeight);




}
