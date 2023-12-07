#include "networkmanager.h"

NetworkManager::NetworkManager() :testBuff_(1024){}


void NetworkManager::ConnectToServer(){
    using tcp = boost::asio::ip::tcp;

    tcp::resolver resolver(ioc_);
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
        boost::asio::io_context::work work(ioc_);

        ioc_.run();
        std::cout << "io_context stopped running" << std::endl;
    }).detach();

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

//用于读取登陆回馈验证信息
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

    //把信息由信号携带发送回loginWidget让他处理逻辑
    emit loginResponseReceived(success, QString::fromStdString(message));
}

void NetworkManager::CloseSocket(){
    socket_.close();
}


void NetworkManager::ClearStreambuf(boost::asio::streambuf& streambuf) {
    // 消费掉所有数据
    streambuf.consume(streambuf.size());


}
