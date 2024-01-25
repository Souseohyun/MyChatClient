#ifndef MYCONFIG_H
#define MYCONFIG_H

#include <QObject>

class QApplication;


class MyConfig
{
public:
    static QString strAppPath;

    static QString strDataPath;

    static QString strDatabasePath;
    static QString strHeadPath;





    static QString strUserName;
    static QString strPassWord;
    static QString strHeadFile;
public:
    MyConfig();
    static void InitMyConfig(const QString &appPath);
    static void CheckDirs();
};




#endif // MYCONFIG_H
