#include "myconfig.h"


#include <QDir>
#include <QCoreApplication>
#include <QTime>

//初始化各项路径
QString MyConfig::strAppPath                = "./";
QString MyConfig::strDataPath               = "";
QString MyConfig::strDatabasePath           = "";
QString MyConfig::strHeadPath               = "";
QString MyConfig::strConfPath               = "";
QString MyConfig::strBasePath               = "";

QString MyConfig::strIniFile                = "config.ini";

int     MyConfig::userId                    = -1;
QString MyConfig::strUserName               = "";
QString MyConfig::strPassWord               = "";
QString MyConfig::strHeadFile               = "";


MyConfig::MyConfig() {}

void MyConfig::InitMyConfig(const QString &appPath)
{
    //app路径
    strAppPath          = appPath;
    //数据文件夹，及次级文件夹路径
    strDataPath         = strAppPath + "/Data/";
    strDatabasePath     = strDataPath + "Database/";
    strHeadPath         = strDataPath+ "Heads/";
    strConfPath         = strDataPath + "Conf/";
    strBasePath         = strDataPath + "Base/";

    strIniFile          = strConfPath + "config.ini";

    CheckDirs();
    CreateUserInfo();


}

void MyConfig::CheckDirs()
{

    // 数据文件夹
    QDir dir(strDataPath);
    if (!dir.exists()) {
        dir.mkdir(strDataPath);
#ifdef Q_WS_QWS
        QProcess::execute("sync");
#endif
    }

    //data下属database
    dir.setPath(strDatabasePath);
    if (!dir.exists()) {
        dir.mkdir(strDatabasePath);
#ifdef Q_WS_QWS
        QProcess::execute("sync");
#endif
    }

    //data下属head
    // 头像检测目录
    dir.setPath(strHeadPath);
    if (!dir.exists()) {
        dir.mkdir(strHeadPath);
#ifdef Q_WS_QWS
        QProcess::execute("sync");
#endif
    }

    //data下属base
    dir.setPath(strBasePath);
    if (!dir.exists()) {
        dir.mkdir(strBasePath);
#ifdef Q_WS_QWS
        QProcess::execute("sync");
#endif
    }


}

void MyConfig::CreateUserInfo()
{
    // 写入配置文件
    QSettings settings(strIniFile, QSettings::IniFormat);
    QString strGroups = settings.childGroups().join("");
    if (!QFile::exists(strIniFile) || (strGroups.isEmpty()))
    {
        /*用户信息设置*/
        settings.beginGroup("User");
        settings.setValue("ID", userId);
        settings.setValue("Name", strUserName);
        settings.setValue("Passwd", strPassWord);
        settings.endGroup();

        settings.sync();

    }
#ifdef Q_WS_QWS
    QProcess::execute("sync");
#endif
}

void MyConfig::SaveUserInfo()
{
    QSettings settings(strIniFile, QSettings::IniFormat);

    settings.beginGroup("User");
    settings.setValue("ID", userId);
    settings.setValue("Name", strUserName);
    settings.setValue("Passwd", strPassWord);
    settings.endGroup();

    settings.sync();
}


void MyConfig::Sleep(int sec)
{
    QTime dieTime = QTime::currentTime().addMSecs(sec);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}
