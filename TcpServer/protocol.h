#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef  unsigned int uint;

#define REGIST_OK "regist OK"
#define REGIST_FAILED "regist failed : name existed"

#define LOGIN_OK "login OK"
#define LOGIN_FAILED "login failed : name error or pwd error or relogin"

#define SEARCH_USR_NO "no such people"
#define SEARCH_USR_ONLINE "online"
#define SEARCH_USR_OFFLINE "offline"

#define UNKNOW_ERROR "add friend error"
#define EXISTED_FRIEND "firend exist"
#define ADD_FRIEND_OFFLINE "usr offline"
#define ADD_FRIEND_NO_EXIST "usr not exist"

#define DEL_FRIEND_OK "delet friend OK"

#define DIR_NO_EXIST "cur dir not exist"
#define FILE_NAME_EXIST "file name exist"
#define CREAT_DIR_OK "create dir ok"

#define DEL_DIR_OK "delete dir ok"
#define DEL_DIR_FAILED "delete dir failed"

#define RENAME_FILE_OK "rename file ok"
#define RENAME_FILE_FAILED "rename file failed"

#define UPLOAD_FILE_OK "upload file ok"
#define UPLOAD_FILE_FAILED "upload file failed"

#define ENTER_DIR_FAILED "enter fir failed: is regular file"

#define DEL_FILE_OK "delete file ok"
#define DEL_FILE_FAILED "delete file failed"

#define MOVE_FILE_OK "move file ok"
#define MOVE_FILE_FAILED "move file failed"

#define COMMON_ERROR "operate faile:system busy"

enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST,
    ENUM_MSG_TYPE_REGIST_RESPOND,

    ENUM_MSG_TYPE_LOGIN_REQUEST,//登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND,

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,//在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,

    ENUM_MSG_TYPE_SEARCH_USR_REQUEST,//搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND,

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,//添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,

    ENUM_MSG_TYPE_ADD_FRIEND_AGREE,//同意添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,

    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,//刷新好友请求
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND,

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,//删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,//删除好友请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,

    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,//群聊
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,//创建文件夹请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,

    ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,//创建文件夹请求
    ENUM_MSG_TYPE_FLUSH_FILE_RESPOND,

    ENUM_MSG_TYPE_DEL_DIR_REQUEST,//删除目录
    ENUM_MSG_TYPE_DEL_DIR_RESPOND,

    ENUM_MSG_TYPE_RENAME_FILE_REQUEST,//重命名文件
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND,

    ENUM_MSG_TYPE_ENTER_DIR_REQUEST,//进入文件
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,

    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,//上传文件
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,

    ENUM_MSG_TYPE_DEL_FILE_REQUEST,//删除文件
    ENUM_MSG_TYPE_DEL_FILE_RESPOND,

    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,//下载文件
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND,

    ENUM_MSG_TYPE_SHARE_FILE_REQUEST,//共享文件
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND,

    ENUM_MSG_TYPE_SHARE_FILE_NOTE,
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND,

    ENUM_MSG_TYPE_MOVE_FILE_REQUEST,//移动文件
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND,

    ENUM_MSG_TYPE_MAX = 0x00ffffff
};

struct FileInfo
{
    char caFileName[32];
    int iFileType;
};

struct PDU
{
    uint uiPDULen; //总的协议数据单元大小
    uint uiMsgType; //消息类型
    char caData[64];
    uint uiMsgLen;//实际消息长度
    int caMsg[];//动态申请空间
};

PDU *mkPDU(uint uiMsgLen);
#endif // PROTOCOL_H
