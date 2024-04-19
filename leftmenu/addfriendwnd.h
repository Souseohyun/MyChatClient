#ifndef ADDFRIENDWND_H
#define ADDFRIENDWND_H

#include "chatsession/cell.h"
#include "base/roundlabel.h"
#include "nlohmann/json.hpp"

#include <QDialog>
#include <QLabel>
#include <QPushButton>

class AddFriendWnd : public QDialog
{
    Q_OBJECT

public:
    AddFriendWnd(Cell *cell);

signals:
    //旧
    void signalAddFriend(const quint8 &,const QJsonValue&);
    //新
    void signalAddFriend2(const nlohmann::json &);
    void signalAddChat(Cell*);

public slots:
    void sltBtnClicked();

private:
    RoundLabel *headLabel;
    QLabel *idLabel;
    QLabel *nameLabel;
    QPushButton *agreeBtn;
    QPushButton *rejectBtn;
    QLabel *noticeLabel;
    QFont font;

    Cell *cell;
};

#endif // ADDFRIENDWND_H
