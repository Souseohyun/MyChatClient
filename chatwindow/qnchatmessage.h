#ifndef QNCHATMESSAGE_H
#define QNCHATMESSAGE_H

#include <QWidget>

class QPaintEvent;
class QPainter;
class QLabel;
class QMovie;

class QNChatMessage : public QWidget
{

    Q_OBJECT
public:
    explicit QNChatMessage(QWidget *parent = nullptr);
    explicit QNChatMessage(const QPixmap& rightPixmap, QWidget *parent = nullptr);



    enum User_Type{

        User_System,//系统
        User_Me,    //自己
        User_She,   //用户
        User_Time,  //时间
    };

    enum Msg_Type{
        Msg_Null,
        Msg_Text,
        Msg_Image,
        Msg_File
    };

    void SetHTML(const QString&,User_Type);
    QSize GetSize();
    QSize GetHTMLSize();


    void SetHeaderImage(User_Type userType,QPixmap& headerPic);


    void setTextSuccess();
    void setText(QString text, QString time, QSize allSize, User_Type userType);

    void setHtml(const QString &htmlContent, const QString &time, QSize allSize, User_Type userType);
    void updateGeometry();

    QSize getRealString(QString src);
    QSize fontRect(QString str);

    inline QString text() {
        return m_msg;}
    inline QString time() {
        return m_time;}
    inline User_Type userType() {
        return m_userType;}
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QString m_msg;
    //new demo new html
    QString m_htmlContent;

    QString m_time;
    QString m_curTime;

    QSize m_allSize;
    User_Type m_userType = User_System;
    Msg_Type m_msgType   = Msg_Null;

    int m_kuangWidth;
    int m_textWidth;
    int m_spaceWid;
    int m_lineHeight;

    QRect m_iconLeftRect;
    QRect m_iconRightRect;
    QRect m_sanjiaoLeftRect;
    QRect m_sanjiaoRightRect;
    QRect m_kuangLeftRect;
    QRect m_kuangRightRect;
    QRect m_textLeftRect;
    QRect m_textRightRect;
    QPixmap m_leftPixmap;
    QPixmap m_rightPixmap;
    QLabel* m_loading = Q_NULLPTR;
    QMovie* m_loadingMovie = Q_NULLPTR;
    bool m_isSending = false;
};

#endif // QNCHATMESSAGE_H
