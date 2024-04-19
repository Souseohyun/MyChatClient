#ifndef BUBBLEINFO_H
#define BUBBLEINFO_H

#include <QString>
#include <QDateTime>
#include <QPixmap>



class BubbleInfo
{
public:
    int myid  = 0;
    int srcid = 0;
    int destid= 0;
    bool isSender; // true if the message is from the user, false if from the other person
    bool isGroup;
    //代实现：消息类型

    QString message;

    qint64 time = 0;

    QPixmap avatar;     //待废弃

public:
    BubbleInfo();
};

#endif // BUBBLEINFO_H
