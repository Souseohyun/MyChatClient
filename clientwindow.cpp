#include "clientwindow.h"

#include "myconfig.h"
#include "chatsession/cell.h"
#include "leftmenu/addfriendwnd.h"

#include <QApplication>
#include <QTimer>

ClientWindow::ClientWindow(boost::asio::ip::tcp::socket socket,int& id,QWidget *parent)
    : /*chatWindow(new ChatWindow(std::move(socket),id,this)),*/
    chatNet_(std::move(socket)),
    myId(id),sqlDb(SqlDataBase::Instance()),
    QMainWindow{parent}
{



    //设置无系统边框
    this->setWindowFlags(Qt::FramelessWindowHint);
    //设置透明背景
    this->setAttribute(Qt::WA_TranslucentBackground);
    // 创建底色板
    CreateBackgroundWidget();

    //设置背景颜色setStyleSheet会冲突阴影
    //this->setStyleSheet("background-color: rgb(245, 245, 245);");
    this->setStyleSheet("QMainWindow { background-color: rgb(245, 245, 245); }");

    //创建无边框窗口阴影
    this->CreateShadow();

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

    settings = new SettingsWnd;
    settings->setVisible(false);

    connect(leftBar->settings,&QPushButton::clicked,this,&ClientWindow::SettingShow);
    //创建搜索栏
    searchBar = new SearchBar(nullptr,QSize(280,40));
    searchBar->setFixedWidth(185);

    addMenu = new QMenu(this);
    addMenu->addAction(tr("添加好友"));
    addMenu->addAction(tr("添加群"));
    addMenu->addSeparator();
    addMenu->addAction(tr("创建群"));
    this->SetAddMenuStyle();

    connect(addMenu, &QMenu::triggered, this, &ClientWindow::sltMenuSelected);

    //添加+按钮
    QStringList list;
    list << ":/Icons/MainWindow/add.png"
         << ":/Icons/MainWindow/add.png"
         << ":/Icons/MainWindow/add.png";
    MyButton *addBtn = new MyButton(nullptr,list,QSize(30,30));
    connect(addBtn,&MyButton::clicked,[&](){
        addMenu->exec(QCursor::pos());
    });


    QHBoxLayout *search_layout = new QHBoxLayout;
    search_layout->addWidget(searchBar);
    search_layout->addSpacing(10);
    search_layout->addWidget(addBtn);
    search_layout->setContentsMargins(5,5,5,5);

//
    // 创建一个QWidget作为中心小部件，用来容纳会话列表和联系人列表
    centerWidget = new QStackedWidget(this);
//    QVBoxLayout* centerLayout = new QVBoxLayout(centerWidget);

    //创建群聊的窗口初始化
    newGroup = new CreateGroupWnd;
    connect(newGroup,&CreateGroupWnd::signalCreateGroup,
            this,&ClientWindow::sltCreateGroup);
    connect(&chatNet_,&NetworkManager::createGroupReplyReceived,
            this,&ClientWindow::sltCreateGroupRelpy);
    newGroup->hide();


    // 初始化会话列表和联系人列表，放入小部件centerwidget中管理
    sessionListWidget = new ListWidget;

    contactsListWidget = new ContactsListWidget;

    /*
    sss
    contactsListWidget->InitList();
    contactsListWidget->hide(); // 默认隐藏联系人列表
    */
    // 将会话列表和联系人列表添加到布局中
    //centerLayout->addWidget(sessionListWidget);
    //centerLayout->addWidget(contactsListWidget);
    centerWidget->addWidget(sessionListWidget);
    centerWidget->addWidget(contactsListWidget);
    centerWidget->setCurrentIndex(0);   //默认第一页
    centerWidget->setContentsMargins(0,0,0,0);

    //搜索栏相关信号
    connect(searchBar, &SearchBar::editingFinished, this, &ClientWindow::searchSessions);
    connect(searchBar->getCloseBtn(), &MyButton::clicked, this, &ClientWindow::showAllSessions);

    //会话按钮相关信号
    connect(leftBar->getChatList(),&MyButton::clicked,this,&ClientWindow::showSessionListWidget);

    connect(sessionListWidget,&ListWidget::signalOpenDialog,this,&ClientWindow::sltOpenAddFriendWnd);
    connect(sessionListWidget,&ListWidget::popMenuToShow,this,&ClientWindow::setSelectedMenuCell);
    //联系人界面相关信号
    connect(leftBar->getContacts(),&MyButton::clicked,this,&ClientWindow::showContactsListWidget);


    connect(contactsListWidget,&ContactsListWidget::checkinContact,this,&ClientWindow::contactIsClicked);

    // 配置 sessionListWidget
    connect(sessionListWidget, &ListWidget::signalSonSelected, this, &ClientWindow::OnCellViewSonSelected);
    //头像与更新myHead槽
    connect(&imageNet_, &NetworkManager::headerReceived,
            this, &ClientWindow::updateReceivedHeader);
    //图像服务器发来好友图像资源
    connect(&imageNet_, &NetworkManager::image_id_doubleReceived, this, &ClientWindow::downloadInHead);

    connect(&imageNet_,&NetworkManager::iimageReceived,this,&ClientWindow::downloadInHead);
    //服务器传来消息时
    connect(&chatNet_, &NetworkManager::messageReceived, this, &ClientWindow::onMessageReceived);

    //服务器传来消息，有人请求我为好友
    qDebug()<<"建立了addFriend槽函数";
    connect(&chatNet_,&NetworkManager::addFriendReceived,this,&ClientWindow::sltAddRecv);
    //服务器传来消息，乙方回应了我的请求
    connect(&chatNet_,&NetworkManager::addFriendReplyReceived,this,&ClientWindow::sltAddReply);
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
    vLayout->addSpacing(10);//searchbar下移一点点
    vLayout->addLayout(search_layout); // 首先添加搜索栏布局
    vLayout->addWidget(centerWidget); // 然后添加中心部件（包含了sessionlist和contacts等）


    // 将垂直布局添加到水平布局中
    hLayout->addLayout(vLayout);

    centralWidget->setLayout(hLayout);

    // 设置中央部件为 ClientWindow 的中心部件
    setCentralWidget(centralWidget);


    InitSessionList();
    //由于contact依赖的头像在sessionlist中被初始化通常，所以把它放到后面吧~
    contactsListWidget->InitList();
    contactsListWidget->hide(); // 默认隐藏联系人列表

    // 初始化右侧default chatwindow
    CreateChatWindow(0,"");

    // 初始化未读通知
    for (const auto& notification : GlobalStorage::getNotifications()) {
        // 检查通知类型
        std::string type = notification["type"];
        if (type == "addfriend") {
            sltAddRecv(notification);
        } else if (type == "addfriendreply") {
            sltAddReply(notification);
        } else {
            // 处理更多
        }
    }
    GlobalStorage::clearNotifications();

    //设置最小化及关闭button
    CreateButthon();


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
    pixmap.loadFromData(headData);

    QString filename = QString("%1.jpg").arg(MyConfig::userId);
    QString fullPath = MyConfig::strHeadPath + filename;

    // 将图像保存到文件
    if (!pixmap.save(fullPath)) {
        qDebug() << "Failed to save image for user id:" << MyConfig::userId;
        return;
    }

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

void ClientWindow::sltMenuSelected(QAction *ac)
{
    qDebug() << ac->text();
    if(!ac->text().compare(tr("添加好友"))){
        FindFriendWnd *w = new FindFriendWnd(imageNet_,0);

        connect(w, &FindFriendWnd::signalFind, this, &ClientWindow::sltFind);
        connect(&chatNet_,&NetworkManager::SearchReceived,w,&FindFriendWnd::sltFindFriendReply);
        connect(w,&FindFriendWnd::signalAddFriend,this,&ClientWindow::sltAddFriend);



        w->exec();
    }else if(!ac->text().compare(tr("添加群"))){
        FindFriendWnd *w = new FindFriendWnd(imageNet_,1);

        connect(w,SIGNAL(signalFind(const QJsonValue&)),this,
                SLOT(sltFind(const QJsonValue &)));
        connect(this,SIGNAL(signalFindFriendReply(const QJsonValue &)),
                w,SLOT(sltFindFriendReply(const QJsonValue &)));
        connect(w,SIGNAL(signalSendMessage(const quint8 &, const QJsonValue &)),
                this,SIGNAL(signalSendMessage(const quint8 &, const QJsonValue &)));

        w->exec();
    }else if(!ac->text().compare(tr("创建群"))){
        newGroup->exec();
    }
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
             //尝试再次发送请求该图片
             std::string http = imageNet_.as_HttpSearchImageByUserId(session->id);
             imageNet_.SendToImageServer(http);

             MyConfig::Sleep(300);
                 QString retryHeadPath = MyConfig::strHeadPath + QString::number(session->id) + ".jpg";
                 QFileInfo retryFileInfo(retryHeadPath);
                 if (retryFileInfo.exists()) {
                     //刷新显示左侧list
                     session->iconPath = retryHeadPath;
                     sessionListWidget->refreshList();
                 } else {
                     qDebug() << "";
                     }



         }
        if (session->type == Cell_FriendChat || session->type == Cell_GroupChat) {
            //sessionListWidget->addSonItem(session);
            InsertCelltoSessionList(session);
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

        qDebug()<<"new ChatWindow. "<<sonId;
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
    const int margin = 15;

    // 计算每个按钮的位置
    //这个10纯粹是人工手动偏移，我也搞不懂为什么他没有按照width()算出靠边
    int closeButtonX = this->width() - buttonWidth - margin + 15;
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

void ClientWindow::SetAddMenuStyle()
{
    addMenu->setStyleSheet(R"(
    QMenu {
        border: 1px solid gray; /* 设置边框颜色和宽度 */
        background-color: rgb(240,240,240); /* 设置背景颜色 */
    }
    QMenu::item {
        padding: 5px 25px; /* 设置菜单项内边距，根据需要调整 */
        background-color: transparent; /* 设置菜单项背景透明 */
    }
    QMenu::item:selected {
        background-color: lightgray; /* 设置选中菜单项的背景色 */
    }
)");


}

//完成sessionlist的基本初始化
void ClientWindow::InitSessionList()
{
    QMenu *sonMenu = new QMenu(this);
    sonMenu->addAction(tr("会话置顶"));
    sonMenu->addAction(tr("取消置顶"));
    sonMenu->addSeparator();
    sonMenu->addAction(tr("关闭会话"));
    sonMenu->addAction(tr("关闭全部会话"));
    connect(sonMenu,&QMenu::triggered,
            this,&ClientWindow::OnSonMenuSelected);
    sessionListWidget->setSonPopMenu(sonMenu);
    // 初始化会话列表
    testSession();

}

void ClientWindow::OnSonMenuSelected(QAction *action)
{
    if(!action->text().compare(tr("会话置顶"))){
        qDebug() << "会话置顶";
        sessionListWidget->SetCellToTop(selectedMenuCell);
    }else if(!action->text().compare(tr("取消置顶"))){
        qDebug() << "取消置顶";
        sessionListWidget->CancelCellOnTop(selectedMenuCell);
    }else if(!action->text().compare(tr("关闭会话"))){
        qDebug() << "关闭会话";
        if(selectedMenuCell != nullptr){
            QList<Cell*> cells = sessionListWidget->GetAllCells();
            int cnt = cells.size();
            if(cnt == 1){
                //emit openDialog(nullptr);         //唯一一个特殊处理
            }else{
                for(int i = 0;i < cnt;i++){
                    if(cells.at(i) == selectedMenuCell){
                        if(i != cnt-1){             //不是最后一个格子，关闭该格子后打开下一个
                            if(selectedCell != nullptr){
                                sessionListWidget->ResetCellState();
                                cells.at(i+1)->isClicked = true;
                                sessionListWidget->refreshList();

                                //打开新的聊天对话框
                                if(cells.at(i+1)->type != Cell_AddFriend &&
                                    cells.at(i+1)->type != Cell_AddGroup){
                                    CreateChatWindow(cells.at(i+1)->id,cells.at(i+1)->name);

                                }else{
                                    //如果这个格子是通知，那不用管
                                }
                            }
                        }else{//是最后一个格子，关闭该格子后打开上一个
                            if(selectedCell != nullptr){
                                sessionListWidget->ResetCellState();
                                cells.at(i-1)->isClicked = true;
                                sessionListWidget->refreshList();

                                //打开新的聊天对话框
                                if(cells.at(i-1)->type != Cell_AddFriend &&
                                    cells.at(i-1)->type != Cell_AddGroup){
                                    CreateChatWindow(cells.at(i-1)->id,cells.at(i-1)->name);

                                }
                            }
                        }
                    }
                }
            }

            sessionListWidget->RemoveCell(selectedMenuCell);

            int sst = sessionListWidget->GetAllCells().size();
            if(sst == 0){
                showContactsListWidget();
            }
        }
    }else if(!action->text().compare(tr("关闭全部会话"))){
        qDebug() << "关闭全部会话";
        sessionListWidget->RemoveAllCells();
        showContactsListWidget();
        CreateChatWindow(0,"");

    }
}

void ClientWindow::setSelectedMenuCell(Cell *cell, QMenu *)
{
    qDebug() << "__SelectedMenuCell show on cell:" << cell->id << cell->name;
    selectedMenuCell = cell;

}






void ClientWindow::minimizeWindow() {
    this->showMinimized();
}


void ClientWindow::closeWindow() {

    nlohmann::json byebye;
    byebye["type"] = "offline";
    byebye["state"]= "closewindow";

    chatNet_.SendToServer(byebye);
    MyConfig::Sleep(500);
    chatNet_.CloseSocket();

    imageNet_.CloseSocket();

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        widget->close();
    }
    //setQuitOnLastWindowClosed(true);
    QCoreApplication::exit(0);
    //this->close();
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

void ClientWindow::onMessageReceived(const QString &html,const QString& text, int srcId, int destId)
{
    // 确定通告目标 ChatWindow
    int targetId = (destId == myId) ? srcId : destId;
    if (!chatWindows.contains(targetId)) {
        //ChatWindow::tag = false;    //告知其构造函数中的startAsyncLoadMsg，暂先不调用
        QString name = sessionListWidget->GetCellById(targetId)->name;
        CreateChatWindow(targetId, name);
    }
    ChatWindow* targetWindow = chatWindows[targetId];
    //注意，此时第3参数发送时间是权宜填0，待更改
    //第二参数填message权益之举，第一个应为html，第二应为text
    targetWindow->addMessage(html,text,"0:00",QNChatMessage::User_She,srcId);

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

//该函数还处理查找好友时向服务器请求的图像数据
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
    }else{
        qDebug()<<"这显然是一个新朋友，Cells中不存在，我不认识";
        qDebug()<<"目前是search friend 需要的图像数据";
    }

}

void ClientWindow::showContactsListWidget()
{

    //leftbar中被选中变深色,当前按钮取消选中
    leftBar->getContacts()->onBtnClicked();
    leftBar->getChatList()->restoreBtn();
    //sessionListWidget->hide(); // 隐藏会话列表
    //contactsListWidget->show();    // 显示联系人界面
    centerWidget->setCurrentIndex(1);
}

void ClientWindow::showSessionListWidget()
{
    //同理同上
    leftBar->getChatList()->onBtnClicked();
    leftBar->getContacts()->restoreBtn();
    centerWidget->setCurrentIndex(0);
    //contactsListWidget->hide();
    //sessionListWidget->show();
    //sessionListWidget->refreshList();
}

void ClientWindow::sltOpenDialog(Cell *)
{

}

void ClientWindow::contactIsClicked(Cell* c)
{
    /*
    qDebug()<<"____DEBUG:INTO SON ITEM IS CLICKED";
    //contacts界面隐藏，chatlist界面显现
    showSessionListWidget();
    //chatwindow显示对应聊天框
    CreateChatWindow(cell->id,cell->name);
    */
    showSessionListWidget();
    sessionListWidget->ResetCellState();
    //注意cell从联系人列表切换到聊天列表时需要转换cell的类型
    Cell *cell = isIDExist(c->id);
    if(cell == nullptr){//证明聊天列表中没有和该用户的聊天记录
        cell = new Cell;
        cell->id = c->id;
        cell->name = c->name;
        cell->iconPath = c->iconPath;
        if(c->type == Cell_FriendContact){
            cell->type = Cell_FriendChat;
        }else if(c->type == Cell_GroupContact){
            cell->type = Cell_GroupChat;
        }
        InsertCelltoSessionList(cell);
    }
    cell->isClicked = true;

    CreateChatWindow(cell->id,cell->name);  //右侧chatwindow显示
    sessionListWidget->refreshList();

    //emit openDialog(cell);

}

void ClientWindow::sltAddChat(Cell *cell)
{
    if(cell->type == Cell_FriendChat){

        //中栏插入聊天格子
        sessionListWidget->insertCell(cell);
        showSessionListWidget();
        //格子放到最前面
        sessionListWidget->MakeCellToTop_NoTopImage(cell);



        //更新联系人列表
        Cell* c = new Cell;
        c->type = Cell_FriendContact;
        c->groupName = "我的好友";
        c->id = cell->id;
        c->iconPath = cell->iconPath;
        c->name = cell->name;
        c->markname = cell->markname;
        c->isClicked = false;
        c->status = Status::OnLine;
        contactsListWidget->addCell(c);//更新联系人列表

    }else if(cell->type == Cell_GroupChat){


        Cell* c = isIDExist(cell->id);
        if(c == nullptr){
            //说明中栏目前没有此聊天格子
            //中栏插入聊天格子
            sessionListWidget->insertCell(cell);
            showSessionListWidget();

            //待做：右栏聊天窗口可增加一条消息

        }else{
            c->msg = cell->msg;
            c->subTitle = cell->subTitle;

            //刷新聊天窗口的群成员列表
            Cell* newUser = new Cell;
            newUser->type = Cell_FriendChat;
            newUser->id = cell->newUserID;
            newUser->name = cell->newUserName;
            newUser->iconPath = cell->newUserHead;
            newUser->status = Status::OnLine;
            //rightBar->addNewUserToGroupList(cell->id,newUser);
/*
            //右栏聊天窗口增加一条消息
            QJsonObject jsonObj;
            jsonObj.insert("type",Notice);
            jsonObj.insert("noticeType",NewMember);
            jsonObj.insert("id",cell->newUserID);
            jsonObj.insert("msg",c->msg);
            rightBar->msgReceived(c,jsonObj);
            */
        }
    }
}

//拿到待请求查找的json，这是一个向服务器请求某条好友信息的slt
void ClientWindow::sltFind(const nlohmann::json &js)
{
    //就别tm再发信号了，直接写！
    //emit signalSendMessage(FindFriend,json);
    //向chatNet_请求查找
    nlohmann::json json;
    json["type"] = "search_user";
    json["srcid"] = this->myId;
    json["searchid"] = js.value("id",0);


    chatNet_.SendToServer(json);
}

void ClientWindow::sltAddFriend(const nlohmann::json & js)
{
    qDebug()<<"我准备发送好友请求:INTO slt add friend";
    chatNet_.SendToServer(js);
}

//我是乙方，显示一个甲方添加好友的请求到达
void ClientWindow::sltAddRecv(const nlohmann::json &js)
{

        qDebug()<<"INTO SLT ADD REPLY";
        int srcid = js.value("srcid",0);
        QString name = QString::fromStdString(js.value("username",""));
        QString markname = QString::fromStdString(js.value("nickname",""));
        //可以在此处请求来者头像了
        QString head = QString("%1.jpg").arg(srcid);
        std::string http = imageNet_.as_HttpSearchImageByUserId(srcid);
        imageNet_.SendToImageServer(http);
        MyConfig::Sleep(700);//等待半秒

        Cell *cell = new Cell;
        cell->id = srcid;
        cell->iconPath = MyConfig::strHeadPath + head;
        cell->name = name;
        cell->markname = markname;
        cell->subTitle = QDateTime::currentDateTime().toString("hh:mm:ss");
        cell->msg = QString::number(srcid) + "请求添加好友";
        cell->type = Cell_AddFriend;

        InsertCelltoSessionList(cell);
        showSessionListWidget();


}

void ClientWindow::sltOpenAddFriendWnd(Cell *cell)
{
    AddFriendWnd *w = new AddFriendWnd(cell);
    connect(w,&AddFriendWnd::signalAddFriend2,this,&ClientWindow::sltAddResult);
    //connect(w,SIGNAL(signalAddFriend(const quint8 &,const QJsonValue&)),
    //        this,SIGNAL(signalSendMessage(const quint8 &, const QJsonValue &)));
    connect(w,&AddFriendWnd::signalAddChat,this,&ClientWindow::sltAddChat);
    connect(w,SIGNAL(signalAddChat(Cell*)),this,SIGNAL(signalAddChat(Cell*)));

    w->exec();
}

void ClientWindow::sltAddResult(const nlohmann::json &js)
{
    qDebug()<<"我准备回应好友请求:";
    chatNet_.SendToServer(js);
}

//我是甲方，我的好友请求乙方给出了回应
void ClientWindow::sltAddReply(const nlohmann::json &js)
{
    //乙方做出回应
    if(js.value("is_agree",false)){
        SqlDataBase::Instance()->addFriend(MyConfig::userId,
                                           js.value("srcid",0),
                                           QString::fromStdString(js.value("username","")),
                                           "我的好友",
                                           QString::fromStdString(js.value("nickname","")));
        int id = js.value("srcid",0);
        QString filename = QString("%1.jpg").arg(id);

        //通知中栏添加与该用户的聊天格子
        Cell *c = new Cell;
        c->type = Cell_FriendChat;
        c->id = id;
        //暂替之举
        c->name = QString::fromStdString(js.value("nickname",""));
        c->markname = QString::fromStdString(js.value("nickname",""));

        c->iconPath = MyConfig::strHeadPath+filename;
        c->isClicked = false;
        c->subTitle = QDateTime::currentDateTime().toString("hh:mm:ss");
        c->msg = "你和" + QString::number(c->id) + "已经成为好友了！";
        c->status = Status::OnLine;

        //一个函数 加入会话栏 contacts栏
        if(!isIDExist(c->id)){
            sltAddChat(c);
        }else{
            qDebug()<<"你来晚一步，我已经从服务器端用户关系数据库读来了你的好友信息";
        }


    }
}

//向服务器发送创建群
void ClientWindow::sltCreateGroup(const nlohmann::json &js)
{
    //emit signalSendMessage(CreateGroup,js);
    nlohmann::json json;
    json["type"]   = "create_group";
    json["adminID"]= js.value("adminID",0);
    json["name"]   = js.value("name","");
    json["time"]   = js.value("time",qint64());
    chatNet_.SendToServer(json);
}
//服务器处理完你的创建群请求
void ClientWindow:: sltCreateGroupRelpy(const nlohmann::json &js)
{
    qDebug()<<"进入了建群回执槽函数";
    //在服务器端，如果创建失败，返回群号是-1作为鉴别
    int groupId = js.value("groupId",0);
    if(groupId == -1){
        qDebug()<<"创建群聊彻底失败";
        return ;
    }
    std::string groupName = js.value("name","");
    qDebug()<<QString::fromStdString(groupName);
    int adminId = MyConfig::userId;
    //转存本地数据库
    QString headImage = "nullhead";
    sqlDb->createGroup(groupId, QString::fromStdString(groupName), adminId, headImage);

    contactsListWidget->displayNewGroups(groupId);
}

void ClientWindow::SettingShow()
{
    settings->show();
}

Cell *ClientWindow::isIDExist(int id)
{
    QList<Cell*> cells = sessionListWidget->GetAllCells();
    for(int i = 0;i < cells.size();i++){
        if(cells[i]->id == id && (cells[i]->type == Cell_FriendChat ||
                                   cells[i]->type == Cell_GroupChat))
            return cells[i];
    }
    return nullptr;
}

void ClientWindow::InsertCelltoSessionList(Cell *c)
{
    QList<Cell*> cells = sessionListWidget->GetAllCells();
    int cnt = cells.size();
    if(c->type == Cell_AddFriend){
        for(int i = 0;i < cnt;i++){
            if(cells.at(i)->id == c->id && (cells.at(i)->type == Cell_AddFriend ||
                                             cells.at(i)->type == Cell_AddGroup)){
                cells.at(i)->done = false;
                return;
            }
        }

        sessionListWidget->insertCell(c);
    }else if(c->type == Cell_AddGroup){
        for(int i = 0;i < cnt;i++){
            if(cells.at(i)->id == c->id && cells.at(i)->groupid == c->groupid &&
                (cells.at(i)->type == Cell_AddFriend || cells.at(i)->type == Cell_AddGroup)){
                cells.at(i)->done = false;
                return;
            }
        }

        sessionListWidget->insertCell(c);
    }else if(c->type == Cell_FriendChat || c->type == Cell_GroupChat){
        for(int i = 0;i < cnt;i++){
            if(cells.at(i)->id == c->id && (cells.at(i)->type == Cell_FriendChat ||
                                             cells.at(i)->type == Cell_GroupChat)){
                return;
            }
        }

        sessionListWidget->insertCell(c);
    }
}

void ClientWindow::CreateBackgroundWidget()
{
    if (backgroundWidget != nullptr) {
        delete backgroundWidget;
        backgroundWidget = nullptr;
    }

    // 创建底色板
    backgroundWidget =  new BackgroundWidget(QColor(235, 234, 232, 255),this);
    backgroundWidget2=  new BackgroundWidget(QColor(240, 240, 240, 255),this);

    // 获取窗口的几何信息
    QRect geometry = this->geometry();

    // 设置底色板的大小和位置（留出边距）
    int margin = 10;
    backgroundWidget->setGeometry(geometry.x() + margin, geometry.y() + margin ,
                                  geometry.width() - 2 * margin -311/*+ 336*/, geometry.height() - 2 * margin + 171);
    backgroundWidget2->setGeometry(geometry.x()+ margin + 309,
                                   geometry.y() + margin ,
                                   geometry.width() - 2 * margin +28,
                                   geometry.height() - 2 *margin + 171);


    // 设置底色板的布局约束
    backgroundWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    backgroundWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 将底色板设置为最底层
    backgroundWidget->lower();
    backgroundWidget2->lower();
    // 添加底色板到窗口中
    backgroundWidget->show();
    backgroundWidget2->show();
}


