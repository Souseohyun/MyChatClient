#include "mainwindow.h"
#include "loginwidget.h"
#include "chatsession/cell.h"


#include"myconfig.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "MychatClient_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }


    QApplication::setQuitOnLastWindowClosed(false);

    qRegisterMetaType<Cell*>("Cell*");

    //初始化一切配置
    MyConfig::InitMyConfig(a.applicationDirPath());
    //database的创建移交给login，每个user有自己的db
    //SqlDataBase::Instance()->openDb(MyConfig::strDatabasePath + "user.db");
    //SqlDataBase::Instance()->openDb(1004);

    //显示登陆界面
     LoginWidget login;
     login.show();


    return a.exec();
}
