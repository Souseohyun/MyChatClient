#ifndef BUBBLEINFO_H
#define BUBBLEINFO_H

#include <QString>
#include <QDateTime>
#include <QPixmap>

class BubbleInfo
{
public:
    QString senderName;
    QString message;
    QDateTime timestamp;
    QPixmap avatar;  // 添加头像图片
    bool isSender; // true if the message is from the user, false if from the other person

public:
    BubbleInfo();
};

#endif // BUBBLEINFO_H
