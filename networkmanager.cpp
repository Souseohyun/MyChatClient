#include "networkmanager.h"
//不传参，默认设置iocontext
NetworkManager::NetworkManager() :socket_(GetIOC()),testBuff_(1024){}

NetworkManager::NetworkManager(boost::asio::ip::tcp::socket socket)
    :socket_(std::move(socket)),testBuff_(1024)
{
    //move create
}

//传参，使用move移交socket所有权

boost::asio::io_context &NetworkManager::GetIOC(){
    static boost::asio::io_context ioc;
    return ioc;
}

boost::asio::ip::tcp::socket &NetworkManager::GetSocket()
{
    return this->socket_;
}


void NetworkManager::ConnectToServer(){
    using tcp = boost::asio::ip::tcp;

    tcp::resolver resolver(GetIOC());
#ifdef _RELEASE
    auto endpoints = resolver.resolve("122.51.38.77", "23610");
#else
    auto endpoints = resolver.resolve("172.30.229.221", "23610");
#endif

    socket_.async_connect(*endpoints.begin(),
                          [this](const boost::system::error_code& ec)
                          {
                              if(!ec){
                                  // qDebug()<<"connect success";
                                  //ReadFromServer();
                              }else{


                              }
                          });

    //单独起一个线程负责ioc_run()
    std::thread([&](){
        boost::asio::io_context::work work(GetIOC());

        GetIOC().run();
        std::cout << "io_context stopped running" << std::endl;
    }).detach();

}

void NetworkManager::ConnectToServer(std::string &ip, std::string &port)
{

}








//所有发送信息只接受json形式，调用该函数前必须自行打好包
void NetworkManager::SendToServer(const nlohmann::json& jsonMessage){
    // 将 JSON 对象转换为字符串
    std::string message = jsonMessage.dump();

    // 添加 \r\n 作为结束符
    message += "\r\n";

    // 将字符串转换为 QByteArray
    QByteArray data = QByteArray::fromStdString(message);

    // 异步发送数据
    boost::asio::async_write(socket_, boost::asio::buffer(data.data(), data.size()),
                             [this](boost::system::error_code ec, std::size_t /*length*/) {
                                 if (!ec) {
                                     // 数据成功发送
                                 } else {
                                     // 发生错误，可能需要进行错误处理
                                 }
                             }
                             );

}

//仅用于读取登陆回馈验证信息
void NetworkManager::ReceiveServerResponse()
{

    boost::asio::async_read_until(socket_, buff_, "\r\n",
                                  [this](const boost::system::error_code& ec, std::size_t /*bytes_transferred*/) {
                                      if (!ec) {
                                          std::istream responseStream(&buff_);
                                          std::string responseData((std::istreambuf_iterator<char>(responseStream)), std::istreambuf_iterator<char>());
                                          std::cout<<"Rece success"<<std::endl;
                                          ProcessServerResponse(responseData);
                                      } else {
                                          // 错误处理
                                          std::cerr << "Error receiving server response: " << ec.message() << std::endl;
                                      }
                                  }
                                  );
}

void NetworkManager::ProcessServerResponse(const std::string &responseData)
{
    std::cout<<"into ProcessServerResponse"<<std::endl;
    // 解析 JSON 数据
    auto responseJson = nlohmann::json::parse(responseData);
    bool success = responseJson["login_success"];
    std::string message = responseJson["message"];
    std::string user_id = responseJson["user_id"];
    //把信息由信号携带发送回loginWidget让他处理逻辑
    emit loginResponseReceived(success, QString::fromStdString(message),QString::fromStdString(user_id));
}



void NetworkManager::ListeningFromSrv()
{
    boost::asio::async_read_until(socket_, buff_, "\r\n",
                                  [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                      if (!ec) {
                                          std::istream responseStream(&buff_);
                                          std::string responseData((std::istreambuf_iterator<char>(responseStream)), std::istreambuf_iterator<char>());
                                          buff_.consume(bytes_transferred); // 清除已读取的数据

                                          try {
                                              auto receivedJson = nlohmann::json::parse(responseData);
                                              HandleReceivedData(receivedJson);
                                          } catch (const nlohmann::json::exception& e) {
                                              // JSON解析错误
                                              std::cerr << "JSON parsing error: " << e.what() << std::endl;
                                          }
                                      } else {
                                          // 错误处理
                                          std::cerr << "Error receiving data: " << ec.message() << std::endl;
                                      }
                                  }
                                  );
}

void NetworkManager::HandleReceivedData(const nlohmann::json &data)
{
    std::string type = data.value("type", "");
    if (type == "re_message_text") {
        // 处理文本消息
        std::string textMessage = data.value("data", "");

        // 发射信号,提示chatwindow采取动作
        emit messageReceived(QString::fromStdString(textMessage));

        //最后再调用Listen一直监听
        ListeningFromSrv();
    }
    // 添加更多type的处理...
}




void NetworkManager::CloseSocket(){
    socket_.close();
}


void NetworkManager::ClearStreambuf(boost::asio::streambuf& streambuf) {
    // 消费掉所有数据
    streambuf.consume(streambuf.size());


}
