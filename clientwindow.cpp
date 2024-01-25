#include "clientwindow.h"

#include "myconfig.h"

ClientWindow::ClientWindow(boost::asio::ip::tcp::socket socket,int& id,QWidget *parent)
    : /*chatWindow(new ChatWindow(std::move(socket),id,this)),*/
    chatNet_(std::move(socket)),
    myId(id),sqlDb(SqlDataBase::Instance()),
    QMainWindow{parent}
{
    //设置无系统边框
    this->setWindowFlags(Qt::FramelessWindowHint);
    //设置透明背景
    //this->setAttribute(Qt::WA_TranslucentBackground);
    //设置背景颜色setStyleSheet会冲突阴影
    //this->setStyleSheet("background-color: rgb(245, 245, 245);");
    this->setStyleSheet("QMainWindow { background-color: rgb(245, 245, 245); }");

    //创建无边框窗口阴影
    //this->CreateShadow();

    //打开数据库连接
    sqlDb->openDb(myId);
#ifdef _RELEASE
    imageNet_.ConnectToServer("122.51.38.77","23611");
#else
    imageNet_.ConnectToServer("172.30.229.221","23611");
#endif
    chatNet_.ListeningFromChatSrv();

    setWindowTitle("Client Window");
    this->setMaximumSize(960, 700);

    resize(960, 640);



    leftBar = new LeftBar(id, this);


    //创建搜索栏
    searchBar = new SearchBar(nullptr,QSize(280,40));
    searchBar->setFixedWidth(185);

    //添加+按钮
    QStringList list;
    list << ":/Icons/MainWindow/add.png"
         << ":/Icons/MainWindow/add.png"
         << ":/Icons/MainWindow/add.png";
    MyButton *addBtn = new MyButton(nullptr,list,QSize(30,30));

    QHBoxLayout *search_layout = new QHBoxLayout;
    search_layout->addWidget(searchBar);
    search_layout->addSpacing(10);
    search_layout->addWidget(addBtn);
    search_layout->setContentsMargins(5,5,5,5);



    //设置左侧会话栏
    sessionListWidget = new ListWidget(this);

    //搜索栏相关信号
    connect(searchBar, &SearchBar::editingFinished, this, &ClientWindow::searchSessions);
    connect(searchBar->getCloseBtn(), &MyButton::clicked, this, &ClientWindow::showAllSessions);

    // 配置 sessionListWidget
    connect(sessionListWidget, &ListWidget::signalSonSelected, this, &ClientWindow::OnCellViewSonSelected);
    //头像与更新myHead槽
    connect(&imageNet_, &NetworkManager::headerReceived,
            this, &ClientWindow::updateReceivedHeader);
    //图像服务器发来好友图像资源
    connect(&imageNet_, &NetworkManager::image_id_doubleReceived, this, &ClientWindow::downloadInHead);

    //服务器传来消息时
    connect(&chatNet_, &NetworkManager::messageReceived, this, &ClientWindow::onMessageReceived);



    std::unique_lock<std::mutex> lock(imageNet_.GetMutex());
    imageNet_.GetCond().wait(lock, [this] { return imageNet_.isConnect_; }); // 等待连接建立

    //一个来回的http交互，目的是拿到自身头像myHead
    std::string http = imageNet_.as_HttpGetImageByUserId(id);
    imageNet_.SendToImageServer(http);
    //该RecvMyheadImageSrv的完成回调中，链式包含请求所有图像的函数
    qDebug()<<"now myid ="<<myId;
    imageNet_.RecvMyheadImageSrv(myId);



    // 创建中央部件和布局
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *hLayout = new QHBoxLayout(centralWidget);
    QVBoxLayout *vLayout = new QVBoxLayout;

    // 首先将 LeftBar 添加到水平布局中
    hLayout->addWidget(leftBar);

    vLayout->addLayout(search_layout); // 首先添加搜索栏布局
    vLayout->addWidget(sessionListWidget); // 然后添加会话列表

    // 添加 sessionListWidget 到水平布局
    //**hLayout->addWidget(sessionListWidget);

    // 设置中央部件的布局
    //***centralWidget->setLayout(hLayout);

    // 将垂直布局添加到水平布局中
    hLayout->addLayout(vLayout);

    centralWidget->setLayout(hLayout);

    // 设置中央部件为 ClientWindow 的中心部件
    setCentralWidget(centralWidget);



    // 初始化会话列表
    testSession();

    // 初始化右侧default chatwindow
    CreateChatWindow(0,"");

    //设置最小化及关闭button
    CreateButthon();

    //button 需要显示到最上层，不被chatwindow给覆盖
    //minimizeButton->raise();
    //closeButton->raise();
}

void ClientWindow::mousePressEvent(QMouseEvent *event)
{
    isPressedWidget = true; // 当前鼠标按下的即是QWidget而非界面上布局的其它控件
    last = event->globalPosition().toPoint(); // 使用 toPoint() 来转换 QPointF 到 QPoint
}

void ClientWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (isPressedWidget)
    {
        QPoint delta = event->globalPosition().toPoint() - last;
        last = event->globalPosition().toPoint();
        move(x() + delta.x(), y() + delta.y());
    }
}

void ClientWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint delta = event->globalPosition().toPoint() - last;
    move(x() + delta.x(), y() + delta.y());
    isPressedWidget = false; // 鼠标松开时，置为false
}


//子会话被选中槽函数
void ClientWindow::OnCellViewSonSelected(int sonId,QString name)
{
    qDebug()<<"into CreateChatWindow---------";
    qDebug()<<"myId:"<<myId<<"  userId:"<<sonId<<"  chatName:"<<name;
    CreateChatWindow(sonId,name);
}

//更新自身头像的槽函数
void ClientWindow::updateReceivedHeader(const QByteArray &headData)
{
    //自身头像交付给clientwindow传入
     QPixmap pixmap;
     if (pixmap.loadFromData(headData)) {
        myHead = pixmap;

     }
}

//searchbar相关槽函数，用来查找相关会话信息并显示，响应editingFinished信号
void ClientWindow::searchSessions()
{
    QString searchTerm = searchBar->text(); // 获取搜索栏的文本
    QList<Cell*> filteredCells; // 过滤后的 Cell 列表

    // 如果搜索栏为空，显示所有会话
    if (searchTerm.isEmpty()) {
        sessionListWidget->refreshList();
        return;
    }

    // 搜索并过滤包含搜索词的 Cell
    for (Cell* cell : sessionListWidget->GetAllCells()) {
        if (cell->name.contains(searchTerm, Qt::CaseInsensitive)) {
            filteredCells.append(cell);
        }
    }

    // 清空当前列表并显示过滤后的会话
    sessionListWidget->clear(); // 清除当前项
    for (Cell* cell : filteredCells) {
        sessionListWidget->addSonItem(cell); // 添加过滤后的项
    }
}

void ClientWindow::showAllSessions()
{
    sessionListWidget->refreshList();
}

//拿到会话头像的逻辑依赖于数据库里存了head的资料
//急需解决的事情是如何批量的将服务器的图像发送到客户端，客户端存到相应位置log by 2024.1.7 21：00
void ClientWindow::testSession() {


    QList<Cell*> sessions = sqlDb->getAllChatSessions();
    if (sessions.isEmpty()) {
        qDebug() << "No chat sessions found or error occurred.";
        return;
    }
     //QPixmap defaultPixmap(":/Icons/MainWindow/default.png"); // 假设这是默认头像的路径
    QString defaultPath = ":/Icons/MainWindow/default.png";
    // 将这些会话添加到 sessionListWidget
    for (Cell* session : sessions) {
         if (!QFileInfo::exists(session->iconPath)) {
             session->iconPath = defaultPath; // 设置默认头像
         }
        if (session->type == Cell_FriendChat || session->type == Cell_GroupChat) {
            sessionListWidget->addSonItem(session);
        } else if (session->type == Cell_FriendDrawer || session->type == Cell_GroupDrawer) {
            qDebug() << "dad item";
        }
    }

    qDebug()<<"sessionListWidget's Size: "<<sessionListWidget->cells.size();
}


void ClientWindow::CreateShadow()
{

    this->setContentsMargins(10, 10, 10, 10);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor("#444444"));
    shadow->setBlurRadius(30);
    this->setGraphicsEffect(shadow);
    this->setContentsMargins(1,1,1,1);

}


void ClientWindow::CreateChatWindow(int sonId,QString chatName)
{



    ChatWindow* chatWindow = nullptr;


    //qDebug()<<"into CreateChatWindow---------";
    if (!chatWindows.contains(sonId)) {
        QString headPath = sqlDb->GetHeadById(sonId);
        QPixmap youHead;
        if (!headPath.isEmpty()) {
            // 如果找到头像路径，加载头像
            youHead.load(MyConfig::strHeadPath + headPath);
        } else {
            // 如果没有找到头像，加载默认头像
            youHead.load(MyConfig::strHeadPath + "default.png");
        }



        // 如果没有为该 userId 创建 ChatWindow，则创建一个
        chatWindow = new ChatWindow(chatNet_,imageNet_,
                                    myId,sonId,chatName,
                                    myHead,youHead,
                                    this);
        chatWindow->setObjectName("chatWindow");
        chatWindow->setFixedSize(648, 631);  // 设置 chatwindow 的固定大小

        qDebug()<<"after new ChatWindow().";
        chatWindows[sonId] = chatWindow;
        chatWindow = chatWindows[sonId];
    }else {
        // 获取已经存在的 ChatWindow
        chatWindow = chatWindows[sonId];
    }

    //连接chatwindow中的信号与槽
    connect(chatWindow, &ChatWindow::messageAdded, this, &ClientWindow::onMessageAdded);

    // 显示新的 ChatWindow
    if (chatWindow != nullptr) {
        QHBoxLayout* layout = dynamic_cast<QHBoxLayout*>(centralWidget()->layout());
        if (layout != nullptr) {
            // 移除当前显示的 ChatWindow
            //ChatWindow 是在布局的第n个位置(此时为3，水平布局为left-session-chatwindow）
            QLayoutItem *currentItem = layout->itemAt(2);
            if (currentItem != nullptr) {
                QWidget* currentWidget = currentItem->widget();
                if (currentWidget != nullptr) {
                    currentWidget->hide();
                    layout->removeWidget(currentWidget);
                }
            }
            qDebug()<<"before layout->addWidget.";
            // 添加新的 ChatWindow
            layout->addWidget(chatWindow);
            qDebug()<<"after layout->addWidget.";
        }

        chatWindow->show();
    }
}

void ClientWindow::CreateButthon()
{
    // 按钮大小 像素
    const int buttonWidth = 24;
    const int buttonHeight = 16;

    // 按钮距离窗口顶部和右侧各 7 像素
    const int margin = 7;

    // 计算每个按钮的位置
    //这个10纯粹是人工手动偏移，我也搞不懂为什么他没有按照width()算出靠边
    int closeButtonX = this->width() - buttonWidth - margin + 10;
    int maximizeButtonX = closeButtonX - buttonWidth - margin;
    int minimizeButtonX = maximizeButtonX - buttonWidth - margin;
    int topButtonX = minimizeButtonX - buttonWidth - margin;

    // 创建和设置顶部按钮
    QPushButton* topButton = new QPushButton(this);
    QPixmap topPixmap(":/Icons/MainWindow/top.png");
    QIcon topButtonIcon(topPixmap.scaled(buttonWidth, buttonHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    topButton->setIcon(topButtonIcon);
    topButton->setFixedSize(buttonWidth, buttonHeight);
    topButton->setGeometry(topButtonX, margin, buttonWidth, buttonHeight);
    topButton->setStyleSheet("QPushButton { border: none; }");

    // 创建和设置最小化按钮
    minimizeButton = new QPushButton(this);
    QPixmap minPixmap(":/Icons/MainWindow/min.png");
    QIcon minimizeButtonIcon(minPixmap.scaled(buttonWidth, buttonHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    minimizeButton->setIcon(minimizeButtonIcon);
    minimizeButton->setFixedSize(buttonWidth, buttonHeight);
    minimizeButton->setGeometry(minimizeButtonX, margin, buttonWidth, buttonHeight);
    minimizeButton->setStyleSheet("QPushButton { border: none; }");

    // 创建和设置最大化按钮
    QPushButton* maximizeButton = new QPushButton(this);
    QPixmap maxPixmap(":/Icons/MainWindow/max.png");
    QIcon maximizeButtonIcon(maxPixmap.scaled(buttonWidth, buttonHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    maximizeButton->setIcon(maximizeButtonIcon);
    maximizeButton->setFixedSize(buttonWidth, buttonHeight);
    maximizeButton->setGeometry(maximizeButtonX, margin, buttonWidth, buttonHeight);
    maximizeButton->setStyleSheet("QPushButton { border: none; }");

    // 创建和设置关闭按钮
    closeButton = new QPushButton(this);
    QPixmap closePixmap(":/Icons/MainWindow/close.png");
    QIcon closeButtonIcon(closePixmap.scaled(buttonWidth, buttonHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    closeButton->setIcon(closeButtonIcon);
    closeButton->setFixedSize(buttonWidth, buttonHeight);
    closeButton->setGeometry(closeButtonX, margin, buttonWidth, buttonHeight);
    closeButton->setStyleSheet("QPushButton { border: none; }");


    // 连接按钮信号到槽函before数
    connect(minimizeButton, &QPushButton::clicked, this, &ClientWindow::minimizeWindow);
    connect(closeButton, &QPushButton::clicked, this, &ClientWindow::closeWindow);
    connect(topButton, &QPushButton::clicked, this, &ClientWindow::topWindow);
    connect(maximizeButton, &QPushButton::clicked, this, &ClientWindow::maximizeWindow);

}






void ClientWindow::minimizeWindow() {
    this->showMinimized();
}

void ClientWindow::closeWindow() {
    this->close();
}

void ClientWindow::maximizeWindow()
{
    if (isMaximized()) {
        // 如果窗口已经是最大化的，则恢复到正常大小
        showNormal();
    } else {
        showMaximized();
    }
}

void ClientWindow::onMessageReceived(const QString &message, int srcId, int destId)
{
    // 确定通告目标 ChatWindow
    int targetId = (destId == myId) ? srcId : destId;
    if (chatWindows.contains(targetId)) {
        ChatWindow* targetWindow = chatWindows[targetId];
        //注意，此时第二参数发送时间是权宜填0，待更改
        targetWindow->addMessage(message,"0:00",QNChatMessage::User_She,srcId);
    }
}

void ClientWindow::topWindow()
{
    // 设置窗口的置顶属性。
    setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
    show(); // 重新显示窗口以应用新的窗口标志
}

void ClientWindow::onMessageAdded(int userId, const QString &lastMessage)
{
    sessionListWidget->RefreshCellTime(userId, QDateTime::currentSecsSinceEpoch(), lastMessage);

}

void ClientWindow::downloadInHead(int id, const QByteArray &imageData)
{
    qDebug()<<"进入下载图像的槽函数中  id: "<<id;

    QPixmap pixmap;
    pixmap.loadFromData(imageData);

    QString filename = QString("%1.jpg").arg(id);
    QString fullPath = MyConfig::strHeadPath + filename;

    // 将图像保存到文件
    if (!pixmap.save(fullPath)) {
        qDebug() << "Failed to save image for user id:" << id;
        return;
    }

    // 更新会话列表中对应的 Cell
    Cell *cellToUpdate = sessionListWidget->GetCellById(id);
    if (cellToUpdate != nullptr) {
        cellToUpdate->iconPath = fullPath; // 更新 Cell 的头像路径
        // 刷新整个会话列表
        //sessionListWidget->refreshList();
        // 更新单个 Cell 的显示
        sessionListWidget->UpdateThisCellDisplay(cellToUpdate);
    }
}


