#ifndef FINDFRIENDWND_H
#define FINDFRIENDWND_H

#include "base/searchbar.h"
#include "base/roundlabel.h"
#include "nlohmann/json.hpp"
#include "networkmanager.h"

#include <QDialog>
#include <QWidget>
#include <QLabel>

class FindFriendWnd : public QDialog
{
    Q_OBJECT

public:
    FindFriendWnd(NetworkManager& net,int tag = 0);

signals:
    void signalFind(const nlohmann::json&);
    //待废弃
    void signalSendMessage(const quint8 &, const QJsonValue &);
    //new
    void signalAddFriend(nlohmann::json&);

    void signalGetHeadFromSrv(const int);

public slots:
    void sltBtnClicked();
    void sltFindFriendReply(const nlohmann::json &);
    void sltAddFriend();

private:
    NetworkManager &imageNet_;

    SearchBar *searchBar;
    QLabel *resultLabel;
    RoundLabel *headLabel;
    QLabel *idLabel;
    QLabel *nameLabel;
    QLabel *msgLabel;

    QPushButton *addBtn;

    int tag;
    int friendID;
};

#endif // FINDFRIENDWND_H
