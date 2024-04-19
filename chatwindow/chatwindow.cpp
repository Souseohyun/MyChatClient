#include "chatwindow.h"

#include "chatwindow/ui_chatwindow.h"
#include "base/emojipicker.h"
#include "sqlite/sqldatabase.h"


#include <QRegularExpression>
#include <QWidgetAction>
#include <QBuffer>

#include <thread>



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
    chatName->setText(name);

    setUIStyle();

    connect(picture, &QPushButton::clicked, this, &ChatWindow::onPictureButtonClicked);

    connect(screenshot,&QPushButton::clicked,this,&ChatWindow::onScreenshotClicked);
    startAsyncLoadMsg();

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


void ChatWindow::addMessage(const QString &html,const QString &text, const QString &time, QNChatMessage::User_Type userType,int addUserId) {
    addX(html,text,time,userType,addUserId);

    //消息写入本地数据库
    BubbleInfo *info = new BubbleInfo;
    if(userType == QNChatMessage::User_Type::User_Me){
        info->isSender = true;
        info->srcid = userId_;
        info->destid= toId_;
    }else{
        info->isSender = false;
        info->srcid = toId_;
        info->destid= userId_;
    }

    info->myid  = userId_;
    info->time = QDateTime::currentSecsSinceEpoch();
    info->isGroup = false;
    info->message = extractContentFromP(html);
    this->SaveMsgToDB(info);

    //info->isSender =
#if 0
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
#endif

}

void ChatWindow::addX(const QString &html,const QString &text, const QString &time, QNChatMessage::User_Type userType,int addUserId,bool isHistoryMessage,bool isLastHistory)
{
    /**/

    qDebug()<<"addX 处理了信息："<<text;

    // 创建一个包含 QLabel 的 QWidget
    QWidget *widget = new QWidget;
    QLabel *label = new QLabel;
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addWidget(label);

    //接下来我们需要扩充，并完成在该Label中显示消息气泡
    QNChatMessage *qncMsg = new QNChatMessage(ui->listWidget);
    //确保qncMsg内部设置各项m_size时的this->width正确而设置该qnc大小
    qncMsg->setFixedWidth(ui->listWidget->width() -  ui->listWidget->contentsMargins().left()); // 为滚动条留出空间
    //头像
    if(userType == QNChatMessage::User_Type::User_Me){
        qncMsg->SetHeaderImage(userType,myPic);
    }else if(userType == QNChatMessage::User_Type::User_She){
        qncMsg->SetHeaderImage(userType,youPic);
    }else{
        qDebug()<<"SetHeaderImage Error";
    }
    qncMsg->SetHTML(html,userType);

    //获取qncMsg显示的气泡确切大小
    QSize msgSize = qncMsg->GetSize();

    qDebug()<<"____DEBUG: now msgSize : "<<msgSize;

    // 将 QWidget 添加到 QListWidget 中
    QListWidgetItem *item = new QListWidgetItem(/*ui->listWidget*/);    //暂不直接加入list

    // 设置大小提示为 QNChatMessage 的推荐大小，并调整以适应 QListWidget 的宽度
    item->setSizeHint(msgSize+QSize(20,20));

    //在加载历史消息的逻辑中，先调用addX的是新消息，故历史消息插入listwidget时，需要每次都插入最上层
    if (isHistoryMessage) {
        // 如果是历史消息，从顶部插入
        ui->listWidget->insertItem(0, item); // 在顶部插入
        ui->listWidget->setItemWidget(item, qncMsg);
        // 不滚动到底部，保持当前滚动位置
    } else {
        // 对于新消息，添加到底部并滚动到底部
        ui->listWidget->addItem(item); // 添加到底部
        ui->listWidget->setItemWidget(item, qncMsg);
        ui->listWidget->scrollToBottom();
    }
/* log 3.15 15:41
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, qncMsg);
    ui->listWidget->scrollToBottom();
*/

    //与左侧会话栏cellviewson的lastMessage交互更新(如果是加载历史消息，那就没必要通知左侧refresh
    if(!isHistoryMessage || isLastHistory){
        if(addUserId == userId_){
            emit messageAdded(toId_, text);
        }else{
            emit messageAdded(addUserId, text);
        }
    }


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

void ChatWindow::setWindowName(const QString &name)
{
    chatName->setText(name);
    chatName->update();
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

#if 0
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
#endif

void ChatWindow::onTextEidtReturnPressed(){
    QString htmlContent = ui->textEdit->toHtml(); // 获取包含文本和图像的 HTML 内容
    QString text        = ui->textEdit->toPlainText().trimmed();
    //此处未注意到表情出现在左侧会话栏目中应该转义为[表情]
    if (!htmlContent.isEmpty()) {
        // 同步显示到 listWidget 控件
        addMessage(htmlContent,text, "12:00", QNChatMessage::User_Me, this->userId_);

        // 创建 JSON 对象
        nlohmann::json json;
        json["type"] = "message_text";
        json["srcid"] = this->userId_;
        json["destid"] = this->toId_;
        json["text"] = htmlContent.toStdString(); // 发送 HTML 内容

        // 发送 JSON 到服务器
        std::string str = json.dump();
        std::cout<<"Send之前\n"<<str<<std::endl;

        chatNetworkManager_.SendToServer(json);

        // 清空发送框 textEdit 控件内容
        ui->textEdit->clear();
    }
}

void ChatWindow::onPictureButtonClicked()
{
    // 弹出系统文件对话框，让用户选择图片文件
    QString imagePath = QFileDialog::getOpenFileName(this, tr("选择图片"), "", tr("Images (*.png *.jpg *.jpeg *.bmp)"));

    // 检查用户是否选择了图片
    if (!imagePath.isEmpty()) {
        // 创建 QPixmap 对象
        QPixmap pixmap(imagePath);

        // 检查图片是否有效
        if (!pixmap.isNull()) {
            // 创建 QTextDocument
            QTextDocument *doc = ui->textEdit->document();

            // 获取当前光标位置
            QTextCursor cursor = ui->textEdit->textCursor();

            // 将 QPixmap 转换为 QTextImageFormat
            QTextImageFormat imageFormat;
            imageFormat.setWidth(100); // 设置图片宽度为100像素
            imageFormat.setHeight(100); // 设置图片高度为100像素
            imageFormat.setName(imagePath); // 设置图片路径

            // 插入图片
            cursor.insertImage(imageFormat);
        } else {
            qDebug() << "无效的图片文件:" << imagePath;
        }
    }
}

void ChatWindow::onScreenshotClicked()
{
    qDebug()<<"onScreen被触发";
    ScreenWidget::Instance()->showFullScreen();
}

void ChatWindow::handleScreenshot(const QPixmap &screenshot)
{
    // 生成截图文件名
    QDateTime currentTime = QDateTime::currentDateTime();
    QString filename = QString("screenshot_%1.png").arg(currentTime.toString("yyyyMMdd_hhmmss"));

    // 构建完整的文件路径
    QString filePath = MyConfig::strBasePath + "/" + filename;

    // 保存截图到计算机中
    bool success = screenshot.save(filePath, "PNG");
    if (success) {
        qDebug() << "Screenshot saved to:" << filePath;
        // 在聊天窗口中显示截图
        QString imgTag = QString("<img src='%1' />").arg(filePath);
        ui->textEdit->insertHtml(imgTag);
        ui->textEdit->moveCursor(QTextCursor::End);
    } else {
        qDebug() << "Failed to save screenshot handle";
    }
}


//
void ChatWindow::setUIStyle(){
    //发送按钮
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

    // 设置 listWidget 和 textEdit 的样式，使其拥有细微的灰色边缘线
    QString subtleBorderStyle = "1px solid #dcdcdc"; // 可以调整颜色和宽度以更好地匹配您的UI风格

    ui->listWidget->setStyleSheet(QString("QListWidget {"
                                          "background-color: rgb(240, 240, 240);"
                                          "border: %1;"
                                          "border-radius: 4px;" // 如果您想要圆角可以调整此值
                                          "}").arg(subtleBorderStyle));
/*
    ui->textEdit->setStyleSheet(QString("QTextEdit {"
                                        "border: %1;"
                                        "border-radius: 4px;"
                                        "}").arg(subtleBorderStyle));

*/
    ui->textEdit->setStyleSheet(
        "QTextEdit {"
        "border: none;" // 移除所有边框
        //"border-radius: 4px;" // 设置边框圆角
        "background-color: rgb(240, 240, 240);" // 设置背景颜色
        "}");

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

void ChatWindow::SaveMsgToDB(BubbleInfo *bf)
{
    SqlDataBase::Instance()->addHistoryMsg(bf);
}



//返回不包含p标签
QString ChatWindow::extractContentFromP(const QString &html)
{
    QRegularExpression regex("<p.*?>(.*?)</p>");
    QRegularExpressionMatch match = regex.match(html);
    if (match.hasMatch()) {
        return match.captured(1); // 获取第一个捕获组的内容，即<p>标签内的内容
    }
    return QString(); // 如果没有匹配，返回一个空的QString
}

QString ChatWindow::RecreateHtml(const QString &ptick)
{
    QString htmlTemplate = R"(
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html>
<head>
    <meta name="qrichtext" content="1" />
    <meta charset="utf-8" />
    <style type="text/css">
        p, li { white-space: pre-wrap; }
        hr { height: 1px; border-width: 0; }
        li.unchecked::marker { content: "\2610"; }
        li.checked::marker { content: "\2612"; }
    </style>
</head>
<body style=" font-family:'Microsoft YaHei UI'; font-size:9pt; font-weight:400; font-style:normal;">
    <p style=" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">%1</p>
</body>
</html>
)";

    return htmlTemplate.arg(ptick);
}

QString ChatWindow::RemoveHtmlTags(const QString &input)
{
    QString output = input;
    // 使用正则表达式匹配并删除所有<...>之间的内容，包括尖括号本身
    QRegularExpression regex("<[^>]*>");
    output.replace(regex, "");
    return output;
}
#include <stack>

void ChatWindow::startAsyncLoadMsg() {


    std::thread([this]() {
        //QVector<MessageData> messagesData;
        std::stack<MessageData> messagesStack;
        int tag = 0; // 代表非群组
        //传入你要找的id
        QVector<BubbleInfo*> bubbles = SqlDataBase::Instance()->QueryMsgHistory(toId_, tag, 0);
        //从时间上来看，vector中，老消息在前，新消息在后
        //push进stack后，老消息在stack底部，新消息在顶部
        //在后续pop后，先pop出的是新消息
        int cnt = bubbles.size();
        for (int i = 0; i < cnt; i++) {

            MessageData data;
            data.html = RecreateHtml(bubbles.at(i)->message);
            data.text = RemoveHtmlTags(bubbles.at(i)->message);
            qint64 time = bubbles.at(i)->time;
            data.srcId = bubbles.at(i)->srcid;

            qDebug()<<data.text;

            if(data.srcId == userId_)
                data.userType = QNChatMessage::User_Me;
            else
                data.userType = QNChatMessage::User_She;
            // 时间处理
            QDateTime dateTime = QDateTime::fromSecsSinceEpoch(time);
            data.timeString = dateTime.toString("HH:mm");

            messagesStack.push(data); // 将消息压入栈
        }

        QMetaObject::invokeMethod(this, [this,cnt, messagesStack]() mutable { // 注意这里需要 mutable,修改了lambda外的值
                while (!messagesStack.empty()) {
                    const MessageData& data = messagesStack.top();
                    // 由于特殊的数据结构倒腾，时间上来看最新的历史消息最先被加载
                    //(为了加载历史消息时不反复刷新左侧栏，只刷新一次
                    bool isLastHistoryMessage = messagesStack.size() == cnt;//真是精妙
                    addX(data.html, data.text, data.timeString, data.userType, data.srcId, true,isLastHistoryMessage);
                    messagesStack.pop(); // 弹出栈顶元素
                }

            }, Qt::QueuedConnection);
    }).detach();
}
