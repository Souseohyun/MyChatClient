#ifndef GLOBALSTORAGE_H
#define GLOBALSTORAGE_H

#include <QList>
#include <nlohmann/json.hpp>


class GlobalStorage {
public:
    static std::vector<nlohmann::json> notifications;

    static void addNotification(const nlohmann::json& notification) {
        // 其实应该注意线程安全
        notifications.push_back(notification);
    }

    static void clearNotifications() {
        notifications.clear();
    }

    static const std::vector<nlohmann::json>& getNotifications() {
        return notifications;
    }




};


#endif // GLOBALSTORAGE_H
