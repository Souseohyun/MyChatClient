#ifndef CONTACTSLISTWIDGET_H
#define CONTACTSLISTWIDGET_H

#include <QListWidget>
#include "chatsession/listwidget.h"
#include "base/mybutton.h"
#include "sqlite/sqldatabase.h"

#include <QWidget>
#include <QStackedWidget>
#include <QHash>

class ContactsListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit ContactsListWidget(QWidget *parent = nullptr);
    void InitList();
    void addCell(Cell*);
    void deleteFriend(int id);

    void setStyle();
    void moveContactToGroup(const QString& groupName);
    void updateContactGroupInDatabase(int contactId, const QString& newGroupName);
    void deleteContact();
    void leaveGroup();

    void displayNewGroups(int id);
private:
    ListWidget *friendList;
    ListWidget *groupList;
    MyButton *friendBtn;
    MyButton *groupBtn;
    QButtonGroup *btnGroup;
    QStackedWidget *stackWidget;

    QHash<QString,Cell*> hash;//通过分组名找到对应的cell抽屉

    Cell *myGroup = nullptr;

    Cell *popMenuCell = nullptr;
public:
signals:
    void signalSendMessage(const quint8 &, const nlohmann::json &);
    void openDialog(Cell*);
    void checkinContact(Cell*);
    void deleteChat(int id);

public slots:
    void onFriendDadMenuSelected(QAction*);
    void onGroupDadMenuSelected(QAction*);
    void onSonMenuSelected(QAction*);
    void onSonDoubleClicked(Cell*);//双击好友格子
    void onSwitchPage(int);
    void setPopMenuCell(Cell*,QMenu*);
    void sltUpdateFriendList(QString);
};

#endif // CONTACTSLISTWIDGET_H
