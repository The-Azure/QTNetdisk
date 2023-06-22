#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include "privatechat.h"

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);

    resize(400,260);

    loadConfig();

    connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(showConnect()));

    connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(recvMsg()));

    //连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);

}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()//从资源文件中读取IP和端口号
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();
        file.close();

        strData.replace("\r\n"," ");
        QStringList strList = strData.split(" ");

        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
        //qDebug() << "IP"<< m_strIP << "port"<< m_usPort;
    }
    else
    {
        QMessageBox::critical(this, "open config", "open config failed");
    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TcpClient::loginName()
{
    return m_strLoginName;
}

QString TcpClient::curPath()
{
    return m_strCurPath;
}

void TcpClient::setCurPath(QString strCurPath)
{
    m_strCurPath = strCurPath;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}

void TcpClient::recvMsg()
{
    if(!OpeWidget::getInstance().getBook()->getDownloadStatu())
    {
        qDebug() <<m_tcpSocket.bytesAvailable();
        uint uiPDULen = 0;
        m_tcpSocket.read((char*)&uiPDULen,sizeof(uint));//第一个参数是存放数据的位置，第二个参数是存放数据的大小
        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        m_tcpSocket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
        switch(pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_RESPOND:
            {
                if(strcmp(pdu->caData, REGIST_OK) == 0)
                {
                    QMessageBox::information(this, "注册", REGIST_OK);
                }
                else if(strcmp(pdu->caData, REGIST_FAILED) == 0)
                {
                    QMessageBox::information(this, "注册",REGIST_FAILED);
                }
                break;
            }
        case ENUM_MSG_TYPE_LOGIN_RESPOND:
            {
                if(strcmp(pdu->caData, LOGIN_OK) == 0)
                {
                    m_strCurPath = QString("./%1").arg(m_strLoginName);
                    QMessageBox::information(this, "登陆", LOGIN_OK);
                    OpeWidget::getInstance().show();
                    this->hide();
                }
                else if(strcmp(pdu->caData, LOGIN_FAILED) == 0)
                {
                    QMessageBox::information(this, "登陆",LOGIN_FAILED);
                }
                break;
            }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
            {
                qDebug() << "客户端收到查询在线消息";
                OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);
                break;
            }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
            {
                qDebug() << "客户端收到查找好友消息";
                if(strcmp(pdu->caData, SEARCH_USR_NO) == 0)
                {
                    QMessageBox::information(this, "搜索", OpeWidget::getInstance().getFriend()->m_strSearchName + "不存在");
                }
                else if(strcmp(pdu->caData, SEARCH_USR_ONLINE) == 0)
                {
                    QMessageBox::information(this, "搜索", OpeWidget::getInstance().getFriend()->m_strSearchName + "在线");
                }
                else{
                    QMessageBox::information(this, "搜索", OpeWidget::getInstance().getFriend()->m_strSearchName + "不在线");
                }
                break;
            }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData + 32, 32);
            int ret = QMessageBox::information(this, "添加好友", QString("%1 want to add you as friend").arg(caName),QMessageBox::Yes, QMessageBox::No);
            PDU *respdu = mkPDU(0);
            memcpy(respdu->caData, pdu->caData, 32);
            memcpy(respdu->caData + 32, pdu->caData + 32, 32);
            if(ret == QMessageBox::Yes)
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
            else
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
        {
            QMessageBox::information(this, "添加好友", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
        {
            QMessageBox::information(this, "添加好友", "添加好友成功");
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            QMessageBox::information(this, "添加好友", "对方拒绝同意您的好友请求");
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
        {
            OpeWidget::getInstance().getFriend()->updataFriendList(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caName[32] = {'\0'};
            memcpy(caName, pdu->caData, 32);
            QMessageBox::information(this, "删除好友", QString("%1删除你作为好友").arg(caName));
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
        {
            QMessageBox::information(this, "删除好友", "删除好友成功");
            OpeWidget::getInstance().getFriend()->flushFriend();
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            if(PrivateChat::getInstance().isHidden())
            {
                PrivateChat::getInstance().show();
            }
            char caSendName[32] = {'\0'};
            memcpy(caSendName, pdu->caData, 32);
            QString strSendName = caSendName;
            PrivateChat::getInstance().setChatName(strSendName);
            PrivateChat::getInstance().updateMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
        {
            qDebug()<<"客户端显示创建文件夹";
            QMessageBox::information(this, "创建文件夹", pdu->caData);
            OpeWidget::getInstance().getBook()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
        {
            qDebug()<<"客户端收到刷新文件请求";
            QString strEnterDir = OpeWidget::getInstance().getBook()->getEnterDir();
            if(!strEnterDir.isEmpty())
            {
                m_strCurPath = strEnterDir;
                OpeWidget::getInstance().getBook()->clearEnterDir();
            }
            OpeWidget::getInstance().getBook()->updateFileList(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
        {
            qDebug()<<"客户端显示删除文件夹";
            QMessageBox::information(this, "删除文件夹", pdu->caData);
            OpeWidget::getInstance().getBook()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:
        {
            qDebug()<<"客户端显示重命名";
            QMessageBox::information(this, "重命名文件", pdu->caData);
            OpeWidget::getInstance().getBook()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
        {
            qDebug()<<"客户端显示进入文件夹失败";
            QMessageBox::information(this, "进入文件夹", pdu->caData);
            OpeWidget::getInstance().getBook()->clearEnterDir();
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
        {
            QMessageBox::information(this, "上传文件",pdu->caData);
            OpeWidget::getInstance().getBook()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_RESPOND:
        {
            qDebug()<<"客户端显示删除文件";
            QMessageBox::information(this, "删除文件", pdu->caData);
            OpeWidget::getInstance().getBook()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
        {
            qDebug()<<"收到服务器返回下载文件答复"<<pdu->caData;
            char caFileName[32] = {'\0'};
            sscanf(pdu->caData, "%s %lld",caFileName, &(OpeWidget::getInstance().getBook()->m_iTotal));
            qDebug()<<"caFileName"<<caFileName<<"m_iTotal"<<OpeWidget::getInstance().getBook()->m_iTotal;
            if(strlen(caFileName) > 0 && OpeWidget::getInstance().getBook()->m_iTotal  > 0)
            {
                OpeWidget::getInstance().getBook()->setDownloadStatu(true);
                m_file.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
                if(!m_file.open(QIODevice::WriteOnly))
                     QMessageBox::warning(this, "下载文件", "获得保存文件的路径失败");
                qDebug()<<"savePath:"+OpeWidget::getInstance().getBook()->getSaveFilePath();
            }
            else{
                QMessageBox::warning(this, "下载文件", "下载文件不能为文件夹或者内容不能为空");
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
        {
            QMessageBox::information(this, "共享文件", "共享文件成功");
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE:
        {
            char* pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            char *pos = strrchr(pPath ,'/');
            if(pos!=NULL)
            {
                pos++;
                QString strNote = QString("%1 share file %2: make sure to receive it?").arg(pdu->caData).arg(pos);
                int ret = QMessageBox::question(this, "共享文件", strNote);
                if(QMessageBox::Yes == ret)
                {
                    PDU* respdu = mkPDU(pdu->uiMsgLen);
                    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                    memcpy(respdu->caMsg, pdu->caMsg, pdu->uiMsgLen);
                    QString strName = TcpClient::getInstance().loginName();
                    strcpy(respdu->caData, strName.toStdString().c_str());
                    m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
                    qDebug()<<strName + "客户确认接受共享文件";
                    free(respdu);
                    respdu=NULL;
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:
        {
            QMessageBox::information(this, "移动文件", pdu->caData);
        }
        default:
            break;
        }
        free(pdu);
        pdu = NULL;
    }
    else {
        qDebug()<<"客户端开始接受文件数据";
        QByteArray buffer = m_tcpSocket.readAll();
        m_file.write(buffer);
        Book *pBook = OpeWidget::getInstance().getBook();
        pBook->m_iReced += buffer.size();
        if(pBook->m_iTotal == pBook->m_iReced)
        {
            qDebug()<<"客户端接受文件结束";
            m_file.close();
            QMessageBox::information(this,"下载文件","下载文件成功结束");
        }
        else
        {
            m_file.close();
            QMessageBox::critical(this, "下载文件", "下载文件失败");
        }
        pBook->m_iReced = 0;
        pBook->m_iTotal = 0;
        pBook->setDownloadStatu(false);
    }
}

#if 0
void TcpClient::on_sent_pb_clicked()
{
    QString strMsg = ui->lineEdit->text();
//    qDebug<< strMsg;
    if(!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.size());
        pdu->uiMsgType = 8888;
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(), strMsg.size());//拷贝
        qDebug() << (char*)(pdu->caMsg);
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this,"信息发送","发送信息不能为空");
    }
}
#endif
void TcpClient::on_lineEdit_inputRejected()
{

}

void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        m_strLoginName = strName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32, strPwd.toStdString().c_str(),32);
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else{
        QMessageBox::critical(this, "登陆","登陆失败，用户名或密码为空");
    }
}

void TcpClient::on_cancel_pb_clicked()
{

}

void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32, strPwd.toStdString().c_str(),32);
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else{
        QMessageBox::critical(this, "注册","注册失败，用户名或密码为空");
    }
}
