#ifndef STATUS_H
#define STATUS_H



enum class Status{
    ConnectedHost = 0x01,   //连接服务器成功
    DisConnectedHost,       //否

    LoginSuccess,       // 登录成功
    LoginPasswdError,   // 密码错误

    OnLine,             // 在线
    OffLine,            //离线

    RegisterOk,
    RegisterFailed,

    AddFriendOk,
    AddFriendFailed
};









#endif // STATUS_H
