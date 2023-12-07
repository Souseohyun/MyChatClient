#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <QString>
#include <QObject>

#include <iostream>

class NetworkManager : public QObject
{
    Q_OBJECT        // This macro must be included to use signals and slots
private:
    boost::asio::io_context      ioc_;
    boost::asio::ip::tcp::socket socket_{ioc_};
    boost::asio::streambuf       buff_;

    std::vector<char>       testBuff_;

signals:
    void dataReceived(const QString& data);//已废弃
    void loginResponseReceived(bool success, const QString& message);

public:
    NetworkManager();



    void ConnectToServer();
    void ReadFromServer();
    void SendToServer(const nlohmann::json& jsonMessage);
    void ReceiveServerResponse();
    void ProcessServerResponse(const std::string& responseData);

    void CloseSocket();

private:
    void ClearStreambuf(boost::asio::streambuf& streambuf);

};

#endif // NETWORKMANAGER_H
