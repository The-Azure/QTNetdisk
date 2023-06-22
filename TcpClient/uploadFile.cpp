#include "uploadFile.h"
#include "tcpclient.h"



uploadFile::uploadFile()
{
    setAutoDelete(true);
}

QString uploadFile::getCurPath()
{
       curPath = TcpClient::getInstance().curPath();
       return curPath;
}

void uploadFile::run()//建立连接，发送头文件，发送文件内容。
{
    QTcpSocket fileSocket;
    fileSocket.connectToHost(QString("127.0.0.1"), 8848);


    filePath = QFileDialog::getOpenFileName();
    if(!filePath.isEmpty())
    {
        int index = filePath.lastIndexOf('/');
        QString strFileName = filePath.right(filePath.size()-index - 1);

        QFile file(filePath);
        qint64 fileSize = file.size(); //获得文件大小

        QString strCurPath = getCurPath();
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        sprintf(pdu->caData, "%s %lld", strFileName.toStdString().c_str(), fileSize);
        qDebug()<<"客户端即将发送pdu";
        qint64 i = fileSocket.write((char*)pdu, pdu->uiPDULen);
        fileSocket.waitForBytesWritten();
        qDebug()<<i;
        free(pdu);
        pdu = NULL;

    }
    QThread::sleep(5);
    QFile file(filePath);
    if(!file.open((QIODevice::ReadOnly)))
    {
        qDebug()<<"文件打开失败";
        return;
    }
    qDebug()<<"客户端准备发送数据";
    char *pBuffer = new char[4096];
    qint64 ret = 0;
    while(true)
    {
        ret = file.read(pBuffer, 4096);
        if(ret > 0&& ret <= 4096)
        {
            fileSocket.write(pBuffer, ret);
            fileSocket.waitForBytesWritten();
        }
        else if(ret == 0)
            break;
        else{
            break;
        }
    }

    qDebug()<<"客户端发送数据完毕";
    file.close();
    delete []pBuffer;

    fileSocket.disconnect();
    fileSocket.close();

}
