#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //连接信号(dataReceived)与槽（displayReceivedData())
    connect(&networkManager_, &NetworkManager::dataReceived, this, &MainWindow::displayReceivedData);
    //连接服务器
    networkManager_.ConnectToServer();

    //焦点设置于pushText
    ui->textEdit->setFocus();

    // 为输入框安装事件过滤器
    ui->textEdit->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//该函数是network类中dataReceived信号的槽，负责将服务器读来的数据append到showText上
void MainWindow::displayReceivedData(const QString& data) {
    ui->showText->append(data); // 假设 showText 是 QTextEdit 控件的 objectName
}


//事件过滤器处理函数
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->textEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            onPushTextReturnPressed();
            return true; // 事件已处理
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::closeEvent(QCloseEvent *event){
    // 确认窗口
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "MyChat",
                                                               tr("确定退出MyChat?\n"),
                                                               QMessageBox::Yes | QMessageBox::No ,
                                                               QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes) {
        networkManager_.CloseSocket();
        event->accept(); // 接受关闭事件，窗口将关闭

    } else {

        event->ignore(); // 忽略关闭事件，窗口不会关闭
    }

}


//该函数是事件过滤器中的空格处理函数，并不是槽
void MainWindow::onPushTextReturnPressed() {
    QString text = ui->textEdit->toPlainText().trimmed();
    if (!text.isEmpty()) {
        // 同步显示到 showText 控件
        ui->showText->append(text);

        // 发送文本给服务器
        //networkManager_.SendToServer(text); // 你一按空格，信息就被发往服务器

        // 清空 pushText 控件内容
        ui->textEdit->clear();
    }
}
