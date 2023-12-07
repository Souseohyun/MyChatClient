#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QMessageBox>

#include "networkmanager.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT



private:
    NetworkManager networkManager_;




public slots:
    void displayReceivedData(const QString& data);

    //该函数是事件过滤器中的空格处理函数，并不是槽
    void onPushTextReturnPressed();


protected:
    //事件过滤器，虚函数必须重写
    bool eventFilter(QObject *obj, QEvent *event) override;
    //窗口关闭不是基础控件，函数需要重载 QWidget 的 closeEvent 函数，需重写
    void closeEvent(QCloseEvent *event) override;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
