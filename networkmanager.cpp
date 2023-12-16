#include "networkmanager.h"
//不传参，默认设置iocontext
NetworkManager::NetworkManager() :socket_(GetIOC()),strand_(GetIOC().get_executor()),testBuff_(1024){}

NetworkManager::NetworkManager(boost::asio::ip::tcp::socket socket)
    :socket_(std::move(socket)),strand_(GetIOC().get_executor()),testBuff_(1024)
{
    //move create
}

//传参，使用move移交socket所有权

boost::asio::io_context &NetworkManager::GetIOC(){
    static boost::asio::io_context ioc;
    return ioc;
}

boost::asio::strand<boost::asio::io_context::executor_type> &NetworkManager::GetGlobalStrand()
{
    static boost::asio::strand<boost::asio::io_context::executor_type> global_strand(GetIOC().get_executor());
    return global_strand;
}

boost::asio::strand<boost::asio::io_context::executor_type> &NetworkManager::GetStrand()
{
    return this->strand_;
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
                                  isConnect_ = true;
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

void NetworkManager::ConnectToServer(const std::string &ip,const std::string &port)
{
    using tcp = boost::asio::ip::tcp;

    tcp::resolver resolver(GetIOC());

    auto endpoints = resolver.resolve(ip, port);

    socket_.async_connect(*endpoints.begin(),
                          [this](const boost::system::error_code& ec)
                          {
                              if(!ec){
                                  std::cout<<"connect success"<<std::endl;
                                  isConnect_ = true;
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
    int user_id = responseJson["user_id"];
    //把信息由信号携带发送回loginWidget让他使用这些数据创建一个chatwindow
    emit loginResponseReceived(success, QString::fromStdString(message),user_id);
}



void NetworkManager::ListeningFromChatSrv()
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
    if (type == "message_text") {
        // 处理文本消息
        std::string textMessage = data.value("data", "");
        int         userId      = data.value("userid",0);

        // 发射信号,提示chatwindow采取动作
        emit messageReceived(QString::fromStdString(textMessage),userId);

        //最后再调用Listen一直监听
        ListeningFromChatSrv();
    }
    // 添加更多type的处理...
}

//外界传来id
std::string NetworkManager::as_HttpGetImageByUserId(int &userId)
{
    QUrl url(QString("http://172.30.229.221:23611/download?id=%1").arg(userId));
    //此时path包括/download?id=1003
    std::string path = url.path().toStdString() + "?" + url.query().toStdString();

    // 构造 HTTP 请求报文
    std::string request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + url.host().toStdString() + "\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";

    //std::cout<<request<<std::endl;



    //debug--


    return request;

}

void NetworkManager::SendToImageServer(std::string &buf)
{

    // 将字符串转换为 QByteArray
    //QByteArray data = QByteArray::fromStdString(buf);

    // 异步发送数据
    boost::asio::async_write(socket_, boost::asio::buffer(buf,buf.size()),
                             [this](boost::system::error_code ec, std::size_t /*length*/) {
                                 if (!ec) {
                                     // 数据成功发送
                                     std::cout<<"send image server success"<<std::endl;
                                 } else {
                                     // 发生错误，可能需要进行错误处理
                                     std::cerr << "Error on send image server: " << ec.message() << std::endl;
                                 }
                             }
                             );


}

void NetworkManager::RecvMyheadImageSrv()
{
    isConnect_ = false;
    std::cout<<"into RecvMyheadImageSrv()"<<std::endl;
    ClearStreambuf(buff_);
    // 读到 "\r\n\r\n" 只是读完了 http 头部
    if(socket_.is_open()){
        std::cout<<"is open"<<std::endl;
    }
    boost::asio::async_read_until(socket_, buff_, "\r\n\r\n",
                                  [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                      if (!ec) {
            std::cout<<"into read until"<<std::endl;
                                          // 使用 std::istream 读取头部
                                          std::istream responseStream(&buff_);
                                          std::string header;
                                          std::string line;
                                          //getline会消耗行末\n，故只能检测line ！=\r
                                          while (std::getline(responseStream, line) && line != "\r") {
                                              header += line + "\n";
                                          }

                                          // 分析 header 拿到 content length
                                          size_t content_length = ParseContentLength(header);

                                          std::cout << "RecvMyheadImage Content length: " << content_length << std::endl;
                                          std::cout << "RecvMyheadImage Header length: " << header.length() << std::endl;

                                          // 计算已经读取的正文长度
                                          size_t already_read_content_length = buff_.size();


                                          // 现在读取剩余的图像数据
                                          boost::asio::async_read(socket_, buff_,
                                                                  boost::asio::transfer_exactly(content_length - already_read_content_length),
                                                                  [this, content_length](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                                                      if (!ec) {
                                                                          // 响应数据现在在 buff_ 中，处理图像数据...
                                                                          std::string image_data(boost::asio::buffers_begin(buff_.data()), boost::asio::buffers_end(buff_.data()));
                                                                          // 清空缓冲区
                                                                          buff_.consume(buff_.size());

                                                                          // 将 std::string 转换为 QByteArray
                                                                          QByteArray qByteArray(image_data.data(), static_cast<int>(image_data.size()));

                                                                          // 发射信号
                                                                          emit headerReceived(qByteArray);
                                                                          isConnect_ = true;


                                                                      } else {
                                                                          // 错误处理
                                                                          std::cerr << "Error receiving image data: " << ec.message() << std::endl;
                                                                      }
                                                                  });
                                      } else {
                                          // 错误处理
                                          std::cerr << "Error receiving header data: " << ec.message() << std::endl;
                                      }
                                  });
}

//由于ImageServer之间交互为http报文，所以重写监听函数是必要的
void NetworkManager::ListeningFromImageSrv()
{
    ClearStreambuf(buff_);
    // 读到 "\r\n\r\n" 只是读完了 http 头部
    boost::asio::async_read_until(socket_, buff_, "\r\n\r\n",
                                  [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                      if (!ec) {
                                          // 使用 std::istream 读取头部
                                          std::istream responseStream(&buff_);
                                          std::string header;
                                          std::string line;
                                          //getline会消耗行末\n，故只能检测line ！=\r
                                          while (std::getline(responseStream, line) && line != "\r") {
                                              header += line + "\n";
                                          }

                                          // 分析 header 拿到 content length
                                          size_t content_length = ParseContentLength(header);

                                          std::cout << "ListeningFromImage Content length: " << content_length << std::endl;
                                          std::cout << "ListeningFromImage Header length: " << header.length() << std::endl;

                                          // 计算已经读取的正文长度
                                          size_t already_read_content_length = buff_.size();

                                          // 现在读取剩余的图像数据
                                          boost::asio::async_read(socket_, buff_,
                                                                  boost::asio::transfer_exactly(content_length - already_read_content_length),
                                                                  [this, content_length](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                                                      if (!ec) {
                                                                          // 响应数据现在在 buff_ 中，处理图像数据...
                                                                          std::string image_data(boost::asio::buffers_begin(buff_.data()), boost::asio::buffers_end(buff_.data()));
                                                                          // 清空缓冲区
                                                                          buff_.consume(buff_.size());

                                                                          // 将 std::string 转换为 QByteArray
                                                                          QByteArray qByteArray(image_data.data(), static_cast<int>(image_data.size()));

                                                                          // 发射信号
                                                                          emit imageReceived(qByteArray);

                                                                          ListeningFromImageSrv();
                                                                      } else {
                                                                          // 错误处理
                                                                          std::cerr << "Error receiving image data: " << ec.message() << std::endl;
                                                                      }
                                                                  });
                                      } else {
                                          // 错误处理
                                          std::cerr << "Error receiving header data: " << ec.message() << std::endl;
                                      }
                                  });
}
//分析header拿到file length
size_t NetworkManager::ParseContentLength(const std::string& header)
{
    std::string searchKey = "Content-Length: ";
    size_t startPos = header.find(searchKey);
    if (startPos != std::string::npos) {
        startPos += searchKey.length(); // 移动到数值的开始位置
        size_t endPos = header.find("\r\n", startPos); // 查找数值结束位置
        if (endPos != std::string::npos) {
            std::string lengthStr = header.substr(startPos, endPos - startPos);
            return std::stoul(lengthStr); // 将字符串转换为无符号长整型
        }
    }
    return 0; // 如果没找到，返回0
}




void NetworkManager::CloseSocket(){
    socket_.close();
}


void NetworkManager::ClearStreambuf(boost::asio::streambuf& streambuf) {
    // 消费掉所有数据
    streambuf.consume(streambuf.size());


}
