#ifndef LEFTBAR_H
#define LEFTBAR_H

#include <QWidget>
#include "base/mybutton.h"
#include "base/roundlabel.h"

class LeftBar : public QWidget
{
    Q_OBJECT
public:
    explicit LeftBar(int id,QWidget *parent = nullptr);

    //MyButton *headIcon;
    RoundLabel *headLabel;

    MyButton *chatList;
    MyButton *contacts;
    MyButton *settings;
    QButtonGroup *m_btnGroup;

    MyButton *getContacts() const;

    MyButton *getChatList() const;

protected:
    // 重写鼠标操作以实现移动窗口
    virtual void mousePressEvent(QMouseEvent *event) override;

signals:

public slots:
};

#endif // LEFTBAR_H
