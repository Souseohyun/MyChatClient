#include "myconfig.h"


#include <QDir>


//初始化各项路径
QString MyConfig::strAppPath                = "./";
QString MyConfig::strDataPath               = "";
QString MyConfig::strDatabasePath           = "";
QString MyConfig::strHeadPath               = "";


MyConfig::MyConfig() {}

void MyConfig::InitMyConfig(const QString &appPath)
{
    //app路径
    strAppPath          = appPath;
    //数据文件夹，及次级文件夹路径
    strDataPath         = strAppPath + "/Data/";
    strDatabasePath     = strDataPath + "Database/";
    strHeadPath         = strDataPath+ "Heads/";

    CheckDirs();

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


}
