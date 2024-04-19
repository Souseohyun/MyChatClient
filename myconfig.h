#ifndef MYCONFIG_H
#define MYCONFIG_H

#include <QObject>
#include <qsettings.h>

class QApplication;


class MyConfig
{
public:
    static QString strAppPath;

    static QString strDataPath;

    static QString strDatabasePath;
    static QString strHeadPath;
    static QString strBasePath;

    static int     userId;
    static QString strUserName;
    static QString strPassWord;
    static QString strHeadFile;

    static QString strConfPath;
    static QString strIniFile;      //配置文件
public:
    MyConfig();
    static void InitMyConfig(const QString &appPath);
    static void CheckDirs();
    //用户信息
    static void CreateUserInfo();
    static void SaveUserInfo();
    //延时
    static void Sleep(int sec);
};




#endif // MYCONFIG_H
