#include "networkmanager.h"
//不传参，默认设置iocontext
NetworkManager::NetworkManager() :socket_(GetIOC()),strand_(GetIOC().get_executor()),testBuff_(1024){}

NetworkManager::NetworkManager(boost::asio::ip::tcp::socket socket)
    :socket_(std::move(socket)),strand_(GetIOC().get_executor()),testBuff_(1024)
{
    //move create
}

std::string NetworkManager::readHttpHeader(std::istream &stream)
{
    //消耗掉可能有的，莫名其妙的包头回车空格等等
    char nextChar = stream.peek();
    while (nextChar == '\r' || nextChar == '\n' || nextChar == ' ') {
        stream.get(); // Remove the character
        nextChar = stream.peek(); // Peek at the next one without removing it
    }

    std::string header;
    char c;
    while (stream.get(c)) { // 读取每个字符
        header.push_back(c);
        if (c == '\n') { // 检查是否读到了一个新行
            char nextChar = stream.peek(); // 查看下一个字符
            if (nextChar == '\r') { // 如果下一个字符是 '\r'，则检查它之后的字符
                stream.get(c); // 读取 '\r'
                header.push_back(c);
                nextChar = stream.peek();
                if (nextChar == '\n') { // 如果这之后是 '\n'，则到达头部结束
                    stream.get(c); // 读取 '\n'
                    header.push_back(c);
                    break; // 跳出循环
                }
            }
        }
    }
    return header;
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

std::mutex &NetworkManager::GetMutex()
{
    return this->netMutex_;
}

std::condition_variable &NetworkManager::GetCond()
{
    return this->netCond_;
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
                          boost::asio::bind_executor(GetStrand(),
                          [this](const boost::system::error_code& ec)
                          {
                              if(!ec){
                                  std::cout<<"connect success"<<std::endl;
                                  {
                                      std::lock_guard<std::mutex> lock(netMutex_);
                                      isConnect_ = true;
                                  }
                                  netCond_.notify_one(); // 通知条件变量

                              }else{


                              }
                                                     })
                          );

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
/*log 2024.1.7 16:12
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
*/
void NetworkManager::ProcessServerResponse(const std::string &responseData) {
    std::cout << "into ProcessServerResponse" << std::endl;
    std::cout<<responseData<<std::endl;
    // 解析 JSON 数据
    auto responseJson = nlohmann::json::parse(responseData);
    bool success = responseJson["login_success"];
    std::string message = responseJson["message"];
    int user_id = responseJson["user_id"];

    // 检查是否有好友信息
    if (responseJson.contains("friends") && responseJson["friends"].is_array()) {
        auto friendsJson = responseJson["friends"];


        // 发送包含好友信息的信号
        emit loginResponseReceivedWithFriends(success, QString::fromStdString(message), user_id, friendsJson);

    } else {
        // 如果没有好友信息，只发送基本登录响应
        emit loginResponseReceived(success, QString::fromStdString(message), user_id);

    }
}

/*
    if (responseJson.contains("friends") && responseJson["friends"].is_array()) {
        auto friendsJson = responseJson["friends"];

        for (const auto& friendJson : friendsJson) {
            std::cout << "Friend ID: " << friendJson.value("friend_id", 0) << std::endl;
            if (friendJson.contains("markname") && !friendJson["markname"].is_null()) {
                std::cout << "Markname: " << friendJson["markname"].get<std::string>() << std::endl;
            } else {
                std::cout << "Markname is missing or null" << std::endl;
            }
            if (friendJson.contains("teamname") && !friendJson["teamname"].is_null()) {
                std::cout << "Teamname: " << friendJson["teamname"].get<std::string>() << std::endl;
            } else {
                std::cout << "Teamname is missing or null" << std::endl;
            }
        }
    }
*/

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
        int         srcid      = data.value("srcid",0);
        int         destid     = data.value("destid",0);

        // 发射信号,提示clientwindow找到合适的chatwindow进行显示
        emit messageReceived(QString::fromStdString(textMessage),srcid,destid);

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
    request += "Connection: keep-alive\r\n";
    request += "\r\n";

    //std::cout<<request<<std::endl;

    return request;

}

void NetworkManager::SendToImageServer(std::string &buf)
{

std::cout << "Attempting to send data to image server" << std::endl;
    // 将字符串转换为 QByteArray
    //QByteArray data = QByteArray::fromStdString(buf);

    // 异步发送数据
    boost::asio::async_write(socket_, boost::asio::buffer(buf,buf.size()),
                             boost::asio::bind_executor(GetStrand(),
                             [this](boost::system::error_code ec, std::size_t /*length*/) {
                                 if (!ec) {
                                     // 数据成功发送
                                     std::cout<<"send image server success"<<std::endl;
                                     {
                                         if(isHead_){}

                                     }
                                 } else {
                                     // 发生错误，可能需要进行错误处理
                                     std::cerr << "Error on send image server: " << ec.message() << std::endl;
                                 }
                             })
                             );


}

void NetworkManager::RecvMyheadImageSrv(int myId)
{
    //int id  = myId;
    std::shared_ptr<int> pid = std::make_shared<int>(myId);
    isHead_ = false;
    std::cout<<"now myid ="<<*pid<<std::endl;

    std::cout<<"into RecvMyheadImageSrv()"<<std::endl;
    ClearStreambuf(buff_);
    // 读到 "\r\n\r\n" 只是读完了 http 头部
    if(socket_.is_open()){
        std::cout<<"RecvMyheadImageSrv socket is open"<<std::endl;
    }
    //不能直接指定strand，我们可以用绑定执行器的方式，bind_executor极为重要
    boost::asio::async_read_until(socket_, buff_, "\r\n\r\n",
                                  boost::asio::bind_executor(GetStrand(),
                                  [this,pid](const boost::system::error_code& ec, std::size_t bytes_transferred) {
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
                                                                  boost::asio::bind_executor(GetStrand(),
                                                                  [this, content_length,pid](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                                                      if (!ec) {
                                                                          std::cout<<"into RecvMyhead async_read"<<std::endl;
                                                                          // 响应数据现在在 buff_ 中，处理图像数据...
                                                                          std::string image_data(boost::asio::buffers_begin(buff_.data()), boost::asio::buffers_end(buff_.data()));
                                                                          // 清空缓冲区
                                                                          buff_.consume(buff_.size());

                                                                          // 将 std::string 转换为 QByteArray
                                                                          QByteArray qByteArray(image_data.data(), static_cast<int>(image_data.size()));

                                                                          // 发射信号,
                                                                          emit headerReceived(qByteArray);
                                                                          //isConnect_ = true;

                                                                          //拿到头像后就开始链式启用requestAllFriendImages
                                                                          requestAllFriendImages(*pid);


                                                                      } else {
                                                                          // 错误处理
                                                                          std::cerr << "Error receiving image data: " << ec.message() << std::endl;
                                                                      }
                                                                  }  )
                                                                  );
                                      } else {
                                          // 错误处理
                                          std::cerr << "Error receiving header data: " << ec.message() << std::endl;
                                      }
                                  }     )
                                  );
}

void NetworkManager::requestAllFriendImages(int id)
{
    std::cout<<"now myid ="<<id<<std::endl;
    std::string request = as_HttpGetAllFriendsImages(id);
    SendToImageServer(request);

    StartReceivingImages();
}

std::string NetworkManager::as_HttpGetAllFriendsImages(int id)
{
    QUrl url(QString("http://172.30.229.221:23611/getAllImages?id=%1").arg(id));
    std::string path = url.path().toStdString() + "?" + url.query().toStdString();

    // 构造 HTTP 请求报文
    std::string request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + url.host().toStdString() + "\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";

    return request;
}

//配合requestAllFriendImages的持续监听请求，保证收到并处理发来的id及对应图像资源
void NetworkManager::StartReceivingImages() {


    ClearStreambuf(buff_);
    if(socket_.is_open()){
        std::cout<<"StartReceivingImages socket is open"<<std::endl;
    }

    std::cout<<"before Start buff_ size(): "<<boost::asio::buffer_size(buff_.data())<<std::endl;
    // 启动一个循环，以异步方式接收图像数据
    boost::asio::async_read_until(socket_, buff_, "\r\n\r\n", // 读取HTTP头部
                                  [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                      if (!ec) {
                                          // 获取streambuf中的数据长度
                                          std::size_t buffer_data_size = boost::asio::buffer_size(buff_.data());
                                          std::cout << "after Start-1 Buffer data size: " << buffer_data_size << std::endl;
                                          std::istream responseStream(&buff_);

                                          //从istream流中拿到包头
                                          std::string header = readHttpHeader(responseStream);

                                          // 分析 header 获取 content length
                                          size_t content_length = ParseContentLength(header);
                                          std::cout<<content_length<<std::endl;
                                          std::cout<<"bytes_transferred: "<<bytes_transferred<<std::endl;
                                          int friendId = ParseFriendId(header); // 假设 header 中包含了 friendId
                                          if(friendId == -1){
                                              std::cout<<"ParseFriendId Error"<<std::endl;
                                          //解析出-2，代表是服务器发来的end信号
                                          }else if(friendId == -2){
                                              std::cout << "Received end of images signal." << std::endl;
                                              return;
                                          }

                                          //检查数据分包状况
                                          if (buff_.size() >= content_length) {
                                              //一次发来并未分包时
                                              std::string image_data(boost::asio::buffers_begin(buff_.data()), boost::asio::buffers_begin(buff_.data()) + content_length);
                                              buff_.consume(content_length);

                                              emit image_id_doubleReceived(friendId, QByteArray::fromStdString(image_data));
                                              // 继续监听
                                              StartReceivingImages();
                                          } else{
                                          // 数据过大产生分包时，读取剩余的图像数据
                                          boost::asio::async_read(socket_, buff_,
                                                                  boost::asio::transfer_exactly(content_length - buff_.size()),
                                                                  [this, friendId](const boost::system::error_code& ec, std::size_t) {
                                                                      if (!ec) {
                                                  std::cout<<"in twice read,friend id: "<<friendId<<std::endl;
                                                                          // 处理图像数据
                                                                          std::string image_data(boost::asio::buffers_begin(buff_.data()), boost::asio::buffers_end(buff_.data()));
                                                                          buff_.consume(buff_.size()); // 清空缓冲区

                                                                          // 发射信号，保存该图片
                                                                          emit image_id_doubleReceived(friendId, QByteArray::fromStdString(image_data));

                                                                          // 继续监听下一个图像
                                                                          StartReceivingImages();
                                                                      } else {
                                                                          std::cerr << "Error receiving image data: " << ec.message() << std::endl;
                                                                      }
                                                                  });
                                          }
                                      } else {
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

//分析header拿到friend-id
int NetworkManager::ParseFriendId(const std::string& header) {
    std::string searchKey = "Friend-Id: ";
    size_t startPos = header.find(searchKey);
    if (startPos != std::string::npos) {
        startPos += searchKey.length(); // 移动到数值的开始位置
        size_t endPos = header.find("\r\n", startPos); // 查找数值结束位置
        if (endPos != std::string::npos) {
            std::string friendIdStr = header.substr(startPos, endPos - startPos);
            try {
                return std::stoi(friendIdStr); // 将字符串转换为整数
            } catch (const std::exception& e) {
                // 转换失败的错误处理
                std::cerr << "Error parsing Friend-Id: " << e.what() << std::endl;
                return -1; // 返回一个代表失败的值
            }
        }
    }
    return -1; // 如果没找到，返回一个代表失败的值
}




void NetworkManager::CloseSocket(){
    socket_.close();
}


void NetworkManager::ClearStreambuf(boost::asio::streambuf& streambuf) {
    // 消费掉所有数据
    streambuf.consume(streambuf.size());


}
