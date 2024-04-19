#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H
#define _RELEASE

#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <QUrl>
#include <QString>
#include <QObject>
#include <QTextDocument>

#include <iostream>
#include <fstream>

#include "globalstorage.h"

struct FriendInfo {
    int friend_id;
    std::string teamname;
    std::string markname;
};

struct UserInfo {
    int user_id;
    std::string username;
    std::string nickname;
    int gender;
    std::string head;
};

class NetworkManager : public QObject
{
    Q_OBJECT        // This macro must be included to use signals and slots
private:
    //转为采用Get IOC()，维系iocontext单例设计模式
    //boost::asio::io_context      ioc_;
    //只共享ioc，其他资源独立，不需要静态锁
    std::condition_variable      netCond_;
    std::mutex                   netMutex_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf       buff_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;//新版强调了执行器概念


    std::vector<char>       testBuff_;

public:
    bool                         isConnect_ {false};
    bool                         isHead_    {true};
    bool                         readyToSend_ {false}; // 初始状态表示还未准备好发送气泡


signals:
    void dataReceived(const QString& data);//已废弃

    //未读消息加载到本地数据库的槽
    void msgResponseReceived(const int,const nlohmann::json&);
    //登录验证的槽（富含好友信息）
    void loginResponseReceived(bool success, const QString& message,int user_id);
     void loginResponseReceivedWithFriends(
        bool success, const QString& message, int user_id,
        const nlohmann::json& friends);

    void messageReceived(const QString& html,const QString& text,int srcId,int destId);
    void headerReceived(const QByteArray& imageData);
    void imageReceived(const QByteArray& imageData);
    void image_id_doubleReceived(int id,const QByteArray& imageData);

    void SearchReceived(const nlohmann::json&);
    //new
    void iimageReceived(int id,const QByteArray& imageData);

    //好友请求
    void addFriendReceived(const nlohmann::json&);

    //乙方了回应好友请求
    void addFriendReplyReceived(const nlohmann::json&);

    //服务器回送注册群聊请求
    void createGroupReplyReceived(const nlohmann::json&);
    //服务器回应了你的注册申请
    void RegisterRelpyRece(const nlohmann::json&);
public:
    NetworkManager();
    NetworkManager(boost::asio::ip::tcp::socket socket);

    std::string readHttpHeader(std::istream& stream);

    void ConnectToServer();
    void ConnectToServer(const std::string& ip,const std::string& port);
    void ReadFromServer();
    void SendToServer(const nlohmann::json& jsonMessage);
    void ReceiveServerResponse();
    void ProcessServerResponse(const std::string& responseData);

    void ListeningFromChatSrv();
    void ReceiveServerData();
    void HandleReceivedData(const nlohmann::json& data);


    //ImageServer专属
    std::string as_HttpGetImageByUserId(int& userId);

    std::string as_HttpSearchImageByUserId(int &userId);
    void SendToImageServer(std::string& buf);
    //仅在chatwindow创建后使用一次，获取自身头像
    void RecvMyheadImageSrv(int myId);

    //请求所有好友图像资源，请求发来完整图像而非路径
    void requestAllFriendImages(int id);
    //打包该特殊请求，包含信息最重要的是：1-标识 2-id
    std::string as_HttpGetAllFriendsImages(int id);

    void StartReceivingImages();
    void ListeningFromImageSrv();

    size_t ParseContentLength(const std::string& str);
    int ParseFriendId(const std::string& header);



    //全服务器管理类共享一个ioc
    static boost::asio::io_context &GetIOC();
    //协调各服务器链接的全局strand
    static boost::asio::strand<boost::asio::io_context::executor_type> &GetGlobalStrand();
    boost::asio::strand<boost::asio::io_context::executor_type> &GetStrand();
    boost::asio::ip::tcp::socket &GetSocket();
    //条件变量与锁
    std::mutex& GetMutex();
    std::condition_variable& GetCond();
    void CloseSocket();

private:
    void ClearStreambuf(boost::asio::streambuf& streambuf);

};



#endif // NETWORKMANAGER_H
