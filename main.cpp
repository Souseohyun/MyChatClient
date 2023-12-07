#include "mainwindow.h"
#include "loginwidget.h"
#include "chatwindow/chatwindow.h"


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
    //MainWindow w;
    //w.show();
    QApplication::setQuitOnLastWindowClosed(false);


     LoginWidget login;
     login.show();

    // ChatWindow c;
    // c.show();


    return a.exec();
}
