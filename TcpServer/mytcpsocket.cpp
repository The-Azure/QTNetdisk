#include "mytcpsocket.h"
#include <QDebug>
#include <stdio.h>
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>
//每次来一个客户端连接TcpServer就会新建一个TcpSocket事务，所以每个客户端对应一个TcpSocket。所以如果事务关系到别的客户端，需要用resend。
MyTcpSocket::MyTcpSocket()
{
    qDebug()<<"1111";
    bool b = connect(this, SIGNAL(readyRead())
            , this, SLOT(recvMsg()));
    qDebug()<<b;
    connect(this, SIGNAL(disconnected())
            , this, SLOT(clientOffline()));
    m_bUpload = false;
    m_pTimer = new QTimer;
    connect(m_pTimer, SIGNAL(timeout())
            ,this ,SLOT(sendFileToClient()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir);
    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();

    QString srcTmp;
    QString destTmp;
    for(int i = 0; i < fileInfoList.size(); i++)
    {
        qDebug()<<fileInfoList[i].fileName();
        srcTmp = strSrcDir + "/" + fileInfoList[i].fileName();
        destTmp = strDestDir + '/' + fileInfoList[i].fileName();
        if(fileInfoList[i].isFile())
        {
            QFile::copy(srcTmp, destTmp);
        }
        else if(fileInfoList[i].isDir())
        {
            if(QString(".") == fileInfoList[i].fileName()
                    ||  QString("..") == fileInfoList[i].fileName())
            {
                continue;
            }
            copyDir(srcTmp, destTmp);
        }
    }
}

void MyTcpSocket::recvMsg()
{
//    qDebug()<<"服务器收到数据包";
    if(!m_bUpload)
    {
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen,sizeof(uint));//第一个参数是存放数据的位置，第二个参数是存放数据的大小
        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        this -> read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
        switch(pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData + 32, 32);
            bool ret = OpeDB::getInstance().handleRegist(caName, caPwd);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
            if(ret)
            {
                qDebug() <<"服务器收到请求登录信息";
                strcpy(respdu->caData, REGIST_OK);
                QDir dir;
                dir.mkdir(QString("../%1").arg(caName));
            }
            else
            {
                strcpy(respdu->caData,REGIST_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData + 32, 32);
            bool ret = OpeDB::getInstance().handleLogin(caName, caPwd);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            if(ret)
            {
                qDebug() <<"服务器收到请求注册信息";
                strcpy(respdu->caData, LOGIN_OK);
                m_strName = caName;
            }
            else
            {
                strcpy(respdu->caData,LOGIN_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            qDebug() << "服务器收到请求查询在线人信息";
            QStringList ret = OpeDB::getInstance().handleAllOnline();
            uint uiMsgLen = ret.size()*32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for(int i = 0; i < ret.size(); i++)
            {
                memcpy(((char*)(respdu->caMsg)) + i*32, ret.at(i).toStdString().c_str(), ret.at(i).size());
                //注意上面需要用char*，因为内存以字节为单位，如果不加的话实际上是32*i*int类型大小的地址
            }
            write((char*)respdu, respdu->uiPDULen);
            free(pdu);
            pdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
        {
            qDebug() << "服务器收到查询用户信息";
            int ret = OpeDB::getInstance().handleSearchUsr(pdu->caData);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
            if(ret == -1)
            {
                strcpy(respdu->caData, SEARCH_USR_NO);
            }
            else if(ret == 1)
            {
                strcpy(respdu->caData, SEARCH_USR_ONLINE);
            }
            else
            {
                strcpy(respdu->caData, SEARCH_USR_OFFLINE);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(pdu);
            pdu = NULL;

            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            qDebug() << "服务器收到添加好友信息";
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32);
            strncpy(caName, pdu->caData + 32, 32);
            int ret = OpeDB::getInstance().handleAddFriend(caPerName,caName);
            PDU *respdu = NULL;
            switch (ret) {
            case -1:
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, UNKNOW_ERROR);
                break;
            case 0:
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, EXISTED_FRIEND);
                break;
            case 1:
                MyTcpServer::getInstance().resend(caPerName, pdu);
                break;
            case 2:
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, ADD_FRIEND_OFFLINE);
                break;
            case 3:
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, ADD_FRIEND_NO_EXIST);
                break;
            default:
                break;
            }
            if(ret != 1)
                write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            pdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
        {
            qDebug() << "服务器收到同意添加好友信息";
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32);
            strncpy(caName, pdu->caData + 32, 32);
            OpeDB::getInstance().handleAddFriendRespond(caPerName,caName);
            PDU *respdu = NULL;
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            qDebug() << "服务器收到拒绝添加好友信息";
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData + 32, 32);
            PDU *respdu = NULL;
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            write((char*)respdu, respdu->uiPDULen);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            QStringList ret = OpeDB::getInstance().handleFlushFriend(caName);
            uint uiMsgLen = ret.size() * 32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            for(int i = 0; i < ret.size(); i++)
            {
                memcpy((char*)(respdu->caMsg) + i*32
                       , ret.at(i).toStdString().c_str()
                       , ret.at(i).size());
            }
            write((char*)respdu, respdu->uiPDULen);
            qDebug() << "服务器发送刷新好友列表";
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caSelfName[32] = {'\0'};
            char caFriendName[32] = {'\0'};
            strncpy(caSelfName, pdu->caData, 32);//strncpy对char*型数据进行操作，memcpy对内存进行擦做
            strncpy(caFriendName, pdu->caData + 32, 32);
            if(OpeDB::getInstance().handleDelFriend(caSelfName, caFriendName))
            {
                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
                strcpy(respdu->caData, DEL_FRIEND_OK);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                MyTcpServer::getInstance().resend(caFriendName,pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char caPerName[32] = {'\0'};
            memcpy(caPerName, pdu->caData + 32, 32);
            MyTcpServer::getInstance().resend(caPerName, pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            QStringList onlineFriend = OpeDB::getInstance().handleFlushFriend(caName);
            QString temp;
            for(int i = 0; i< onlineFriend.size(); i++)
            {
                temp = onlineFriend.at(i);
                MyTcpServer::getInstance().resend(temp.toStdString().c_str(),pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QDir dir;
            QString strCurPath = QString("%1").arg((char*)(pdu->caMsg));
            bool ret = dir.exists(strCurPath);
            PDU *respdu;
            if(ret)
            {
                char caNewDir[32] = {'\0'};
                memcpy(caNewDir, pdu->caData + 32, 32);
                QString strNewPath = strCurPath + "/" + caNewDir;
                qDebug()<<strNewPath;
                ret = dir.exists(strNewPath);//检测是否想要创建的文件已经存在
                if(ret)
                {
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData, FILE_NAME_EXIST);
                }
                else
                {
                    dir.mkdir(strNewPath);
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData, CREAT_DIR_OK);
                }
            }
            else
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData, DIR_NO_EXIST);
            }
            qDebug()<<"服务器完成创建文件";
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen);
            qDebug()<<"pCurPath"<<pCurPath;
            QDir dir(pCurPath);
            QFileInfoList fileInfoList = dir.entryInfoList();
            int iFileCount = fileInfoList.size();
            PDU *respdu = mkPDU(sizeof(FileInfo)*iFileCount);
            FileInfo *pFileInfo = NULL;
            QString strFileName;
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            for(int i = 0; i < iFileCount; i++)
            {
                pFileInfo = (FileInfo*)(respdu->caMsg) +  i;
                //respdu->caMsg得到的是一个数组的首地址，通过FileInfo来转化其为fileinfo类型的指针
                strFileName = fileInfoList[i].fileName();
                memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.toUtf8().size());
                if(fileInfoList[i].isDir())
                {
                    pFileInfo->iFileType = 0;
                }
                else if(fileInfoList[i].isFile())
                {
                    pFileInfo->iFileType=1;
                }
            }
            qDebug()<<"服务器完成创刷新文件";
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
        {
            char caName[32] = {'\0'};
            strcpy(caName, pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            QFileInfo fileInfo(strPath);
            qDebug()<<strPath;
            bool ret = true;
            if(fileInfo.isDir())
            {
                QDir dir;
                dir.setPath(strPath);
                dir.removeRecursively();
            }
            else if(fileInfo.isFile())
            {
                ret = false;
            }
            PDU *respdu = NULL;
            if(ret)
            {
                respdu = mkPDU(strlen(DEL_DIR_OK));
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caData, DEL_DIR_OK, strlen(DEL_DIR_OK));
            }
            else
            {
                respdu = mkPDU(strlen(DEL_DIR_FAILED));
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caData, DEL_DIR_FAILED, strlen(DEL_DIR_FAILED));
            }
            qDebug()<<"服务器完成删除文件";
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
        {
            char caOldName[32] = {'\0'};
            char caNewName[32] = {'\0'};
            strncpy(caOldName, pdu->caData, 32);
            strncpy(caNewName, pdu->caData +32, 32);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);

            QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName);

            qDebug()<< strOldPath;
            qDebug()<< strNewPath;

            QDir dir;
            bool ret = dir.rename(strOldPath, strNewPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData, RENAME_FILE_OK);
            }
            else
            {
                strcpy(respdu->caData, RENAME_FILE_FAILED);
            }
            qDebug()<<"服务器完成重命名文件";
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            char caEnterName[32] = {'\0'};
            strncpy(caEnterName, pdu->caData, 32);

            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);

            QString strPath = QString("%1/%2").arg(pPath).arg(caEnterName);
            qDebug()<<"服务器收到进入文件夹信号"<<strPath;
            QFileInfo fileInfo(strPath);
            PDU *respdu = NULL;
            if(fileInfo.isDir())
            {
                QDir dir(strPath);
                QFileInfoList fileInfoList = dir.entryInfoList();
                int iFileCount = fileInfoList.size();
                respdu = mkPDU(sizeof(FileInfo)*iFileCount);
                FileInfo *pFileInfo = NULL;
                QString strFileName;
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                qDebug()<<"服务器发送成功信号";
                for(int i = 0; i < iFileCount; i++)
                {
                    pFileInfo = (FileInfo*)(respdu->caMsg) +  i;
                    //respdu->caMsg得到的是一个数组的首地址，通过FileInfo来转化其为fileinfo类型的指针
                    strFileName = fileInfoList[i].fileName();
                    memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size());
                    if(fileInfoList[i].isDir())
                    {
                        pFileInfo->iFileType = 0;
                    }
                    else if(fileInfoList[i].isFile())
                    {
                        pFileInfo->iFileType=1;
                    }
                }
            }
            else if(fileInfo.isFile())
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy(respdu->caData, ENTER_DIR_FAILED);
                qDebug()<<"服务器发送失败";
            }
            qDebug()<<"服务器完成进入文件";
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            qDebug()<<"服务器收到请求上传文件";
            char caFileName[32] = {'\0'};
            qint64 fileSize = 0;
            sscanf(pdu->caData, "%s %lld", caFileName, &fileSize);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            delete []pPath;
            pPath = NULL;

            m_file.setFileName(strPath);
            //以写方式打开文件，若文件不存在，则自动创建文件
            if(m_file.open(QIODevice::WriteOnly))
            {
                m_bUpload = true;
                m_iTotal = fileSize;
                m_iRecved = 0;
            }
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_REQUEST:
        {
            char caName[32] = {'\0'};
            strcpy(caName, pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            QFileInfo fileInfo(strPath);

            bool ret = true;
            if(fileInfo.isDir())
            {
                ret = false;
            }
            else if(fileInfo.isFile())
            {
                QDir dir;
                ret = dir.remove(strPath);
            }
            PDU *respdu = NULL;
            if(ret)
            {
                respdu = mkPDU(strlen(DEL_DIR_OK));
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caData, DEL_FILE_OK, strlen(DEL_FILE_OK));
            }
            else
            {
                respdu = mkPDU(strlen(DEL_DIR_FAILED));
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caData, DEL_FILE_FAILED, strlen(DEL_FILE_FAILED));
            }
            qDebug()<<"服务器完成删除文件";
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            strcpy(caFileName, pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            delete []pPath;
            pPath = NULL;

            QFileInfo fileInfo(strPath);
            qint64 fileSize = fileInfo.size();
            PDU* respdu =mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            sprintf(respdu->caData, "%s %lld", caFileName, fileSize);

            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu=NULL;

            m_file.setFileName(strPath);
            m_file.open(QIODevice::ReadOnly);
            m_pTimer->start(1000);

            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
        {
            qDebug()<<"服务器收到分享文件请求";
            char caSendName[32];
            int num = 0;
            sscanf(pdu->caData,"%s %d",caSendName, &num);
            int size = num*32;
            PDU *respdu = mkPDU(pdu->uiMsgLen-size);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            strcpy(respdu->caData, caSendName);
            memcpy(respdu->caMsg, (char*)(pdu->caMsg)+size, pdu->uiMsgLen-size);
            qDebug()<<caSendName<< num<<(char*)(respdu->caMsg);
            char caRecvName[32] = {'\0'};
            for(int i = 0; i<num; i++)
            {
                 memcpy(caRecvName, (char*)(pdu->caMsg)+i*32,32);
                 MyTcpServer::getInstance().resend(caRecvName, respdu);
                 qDebug()<<"共享请求发送给:"<< caRecvName;
            }
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:
        {

            QString strRecvPath = QString("./%1").arg(pdu->caData);
            QString strShreFilePath = QString("%1").arg((char*)(pdu->caMsg));
            int index = strShreFilePath.lastIndexOf('/');
            QString strFileName = strShreFilePath.right(strShreFilePath.size() - index - 1);
            strRecvPath = strRecvPath + '/' + strFileName;
            QFileInfo fileInfo(strShreFilePath);
            qDebug()<<strShreFilePath<<strRecvPath;
            if(fileInfo.isFile())
            {
                QFile::copy(strShreFilePath,strRecvPath);
            }
            else if(fileInfo.isDir())
            {
                qDebug()<<"传送目录";
                copyDir(strShreFilePath, strRecvPath);
            }
            PDU *respdu = NULL;
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caData, "share file ok");
            qDebug()<<"回复共享成功pdu";
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            int srcLen = 0;
            int destLen = 0;
            sscanf(pdu->caData, "%d%d%s", &srcLen, &destLen, caFileName);

            char *pSrcPath = new char[srcLen + 1];
            char *pDestPath = new char[destLen + 1 +32];
            memset(pSrcPath, '\0', srcLen +1);
            memset(pDestPath, '\0', srcLen +1);

            memcpy(pSrcPath, pdu->caMsg, srcLen);
            memcpy(pDestPath, (char*)(pdu->caMsg)+srcLen + 1, destLen);

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;

            QFileInfo fileInfo(pDestPath);
            if(fileInfo.isDir())
            {
                strcat(pDestPath, "/");
                strcat(pDestPath, caFileName);
                qDebug()<<pDestPath<<"   "<<caFileName;
                bool ret = QFile::rename(pSrcPath,pDestPath);
                if(ret)
                {
                    strcpy(pdu->caData, MOVE_FILE_OK);
                }
                else
                {
                    strcpy(pdu->caData, COMMON_ERROR);
                }
            }
            else if(fileInfo.isFile())
            {
                strcpy(pdu->caData, MOVE_FILE_FAILED);
            }
            write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu=NULL;
            break;
        }
        default:
            break;
        }
        free(pdu);
        pdu = NULL;
    }
    else
    {
//        qDebug()<<"服务器准备接受文件";
        PDU *respdu = NULL;
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;

        QByteArray buff = readAll();
        m_file.write(buff);
        m_iRecved += buff.size();
        if(m_iTotal == m_iRecved)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData, UPLOAD_FILE_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            qDebug()<<"服务器接受文件完毕";
        }
        else if(m_iTotal < m_iRecved)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData, UPLOAD_FILE_FAILED);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
        }
    }
}

void MyTcpSocket::clientOffline()
{
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);
}

void MyTcpSocket::sendFileToClient()
{
    char *pData = new char[4096];
    qint64 ret = 0;
    int i =0;
    while(true)
    {
        m_pTimer->stop();
        qDebug()<<++i;
        ret = m_file.read(pData, 4096);
        if(ret > 0 && ret <= 4096)
            write(pData,ret);
        else if(ret == 0)
        {
            qDebug()<<"传输内容成功结束";
            m_file.close();
            break;
        }
        else if(ret < 0){
            qDebug()<<"发送文件内容给客户端过程中失败";
            m_file.close();
            break;
        }
    }
    delete []pData;
    pData = 0;
}

