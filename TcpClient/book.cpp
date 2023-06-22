#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QThreadPool>
Book::Book(QWidget *parent) : QWidget(parent)
{
    m_bDownload = false;
    m_pTimer = new QTimer;
    QThreadPool::globalInstance()->setMaxThreadCount(4);
    m_pBookListW = new QListWidget;
    m_pPReturnPB = new QPushButton("返回上一级");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pRenamePB = new QPushButton("重命名");
    m_pFlushFilePB = new QPushButton("刷新目录信息");

    QVBoxLayout *pDirVBL =  new QVBoxLayout;
    pDirVBL->addWidget(m_pPReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    m_pPUploadPB = new QPushButton("上传文件");
    m_pDownLoadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pPShareFilePB = new QPushButton("共享文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectDirPB = new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);

    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pPUploadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pPShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);
    QHBoxLayout *pMain1 = new QHBoxLayout;
    pMain1->addWidget(m_pBookListW);
    pMain1->addLayout(pMain);

    setLayout(pMain1);

    connect(m_pCreateDirPB, SIGNAL(clicked(bool))
            , this, SLOT(createDir()));
    connect(m_pFlushFilePB, SIGNAL(clicked(bool))
            , this, SLOT(flushFile()));
    connect(m_pDelDirPB, SIGNAL(clicked(bool))
            , this, SLOT(delDir()));
    connect(m_pRenamePB, SIGNAL(clicked(bool))
            , this, SLOT(renameFile()));
    connect(m_pBookListW, SIGNAL(doubleClicked(QModelIndex))
            , this, SLOT(enterDir(QModelIndex)));
    connect(m_pPReturnPB, SIGNAL(clicked(bool))
            , this, SLOT(returnPre()));
    connect(m_pPUploadPB, SIGNAL(clicked(bool))
            , this, SLOT(uploadfile()));
    connect(m_pTimer, SIGNAL(timeout())
            , this, SLOT(uploadFileDate()));
    connect(m_pDelFilePB, SIGNAL(clicked(bool))
            , this, SLOT(delRegFile()));
    connect(m_pDownLoadPB, SIGNAL(clicked(bool))
            , this, SLOT(downloadFile()));
    connect(m_pPShareFilePB, SIGNAL(clicked(bool))
            , this, SLOT(ShareFile()));
    connect(m_pMoveFilePB, SIGNAL(clicked(bool))
            , this, SLOT(moveFile()));
    connect(m_pSelectDirPB, SIGNAL(clicked(bool))
            , this, SLOT(selectDestDir()));
}

void Book::updateFileList(const PDU *pdu)
{
    if(NULL == pdu)
    {
        return;
    }
    m_pBookListW->clear();
    FileInfo *pFileInfo = NULL;
    int iCount = pdu->uiMsgLen/sizeof(FileInfo);
    for(int i = 0; i<iCount; i++)
    {
        pFileInfo = (FileInfo*)(pdu->caMsg) + i;
        QListWidgetItem *pItem = new QListWidgetItem;
        if(pFileInfo->iFileType == 0)
        {
            pItem->setIcon(QIcon(QPixmap(":/source/files.png")));
        }
        else
        {
            pItem->setIcon(QIcon(QPixmap(":/source/file.jpg")));
        }
        if(strcmp(pFileInfo->caFileName,".") == 0 ||strcmp(pFileInfo->caFileName, "..") == 0)
            continue;
        pItem->setText(pFileInfo->caFileName);
        m_pBookListW->addItem(pItem);
    }
}

QString Book::getEnterDir()
{
    return m_strEnterDir;
}

void Book::clearEnterDir()
{
    m_strEnterDir.clear();
}

void Book::setDownloadStatu(bool status)
{
    m_bDownload = status;
}

bool Book::getDownloadStatu()
{
    return m_bDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}

void Book::createDir()
{
    QString strNewDir = QInputDialog::getText(this, "新建文件夹", "新文件夹名字");
    if(!strNewDir.isEmpty())
    {
        if(strNewDir.toUtf8().size() <= 32)
        {
            QString strName = TcpClient::getInstance().loginName();
            QString strCurPath = TcpClient::getInstance().curPath();
            PDU *pdu = mkPDU(strCurPath.size() + 1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData, strName.toStdString().c_str(),strName.size());
            strncpy(pdu->caData+32, strNewDir.toStdString().c_str(),strNewDir.toUtf8().size());
            memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
            qDebug()<<"客户端发送当前路径"<<strCurPath;

            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
        else
            QMessageBox::warning(this, "新建文件夹", "名字请小于32个字符");
    }
    else
        QMessageBox::warning(this,"新建文件夹","请输入新文件夹名字");
}

void Book::flushFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)(pdu->caMsg), strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::delDir()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(pItem == NULL)
        QMessageBox::warning(this, "删除文件", "选择不能为空");
    else
    {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.toUtf8().size());
        qDebug()<<"strDelName"<<strDelName;
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::renameFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(pItem == NULL)
        QMessageBox::warning(this, "重命名文件", "选择不能为空");
    else
    {
        QString strOldName = pItem->text();
        QString strNewName = QInputDialog::getText(this, "重命名文件", "请输入新的文件名");
        if(strNewName.isEmpty())
            QMessageBox::warning(this, "重命名文件", "文件名不能为空");
        else
        {
            PDU *pdu = mkPDU(strCurPath.size() + 1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData, strOldName.toStdString().c_str(), strOldName.size());
            strncpy(pdu->caData + 32, strNewName.toStdString().c_str(), strNewName.size());
            memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
    }
}

void Book::enterDir(const QModelIndex &index)
{
    QString strDirName = index.data().toString();
    QString strCurPath = TcpClient::getInstance().curPath();
    m_strEnterDir = QString("%1/%2").arg(strCurPath).arg(strDirName);

    qDebug()<<"收到双击进入文件信号"<<strCurPath;
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData, strDirName.toStdString().c_str(), strDirName.size());
    memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::returnPre()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strRootPath = "./" + TcpClient::getInstance().loginName();
    if(strCurPath == strRootPath)
    {
        QMessageBox::warning(this, "返回", "返回失败：已经处于根目录");
    }
    else
    {
        int index = strCurPath.lastIndexOf('/');
        strCurPath.remove(index, strCurPath.size() - index);
        TcpClient::getInstance().setCurPath(strCurPath);
        qDebug()<< "当前目录路径已经更改";
        flushFile();
    }
}

void Book::uploadfile()
{
    uploadFile *newTask = new uploadFile;
    QThreadPool::globalInstance()->start(newTask);
}

void Book::uploadFileDate()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if(!file.open((QIODevice::ReadOnly)))
    {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }

    char *pBuffer = new char[4096];
    qint64 ret = 0;
    while(true)
    {
        ret = file.read(pBuffer, 4096);
        if(ret > 0&& ret <= 4096)
        {
            TcpClient::getInstance().getTcpSocket().write(pBuffer, ret);
        }
        else if(ret == 0)
            break;
        else{
            QMessageBox::warning(this, "上传文件","上传文件失败：读文件失败");
            break;
        }
    }
    file.close();
    delete []pBuffer;
}

void Book::delRegFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(pItem == NULL)
        QMessageBox::warning(this, "删除文件", "选择不能为空");
    else
    {
        QString strDelName = pItem->text();
        qDebug()<<"strDelName"<<strDelName;
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.toUtf8().size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::downloadFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(pItem == NULL)
        QMessageBox::warning(this, "下载文件", "选择不能为空");
    else
    {
        QString strSaveFilePath = QFileDialog::getSaveFileName();
        if(strSaveFilePath.isEmpty())
        {
            QMessageBox::warning(this, "下载文件", "请指定要保存的位置");
            m_strSaveFilePath.clear();
        }
        else
        {
            m_strSaveFilePath = strSaveFilePath;
        }

        QString strCurPath = TcpClient::getInstance().curPath();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        QString strFileName = pItem->text();
        strcpy(pdu->caData, strFileName.toStdString().c_str());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);

    }
}

void Book::ShareFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(pItem == NULL)
    {
        QMessageBox::warning(this, "分享文件", "选择不能为空");
        return;
    }
    else
    {
        m_strShareFileName = pItem->text();
    }

    Friend *pFriend = OpeWidget::getInstance().getFriend();
    QListWidget *pFriendList = pFriend->getFriendList();
    shareFile::getInstance().updateFriend(pFriendList);
    if(shareFile::getInstance().isHidden())
    {
        shareFile::getInstance().show();
    }
}

void Book::moveFile()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(pCurItem != NULL)
    {
        m_strMoveFileName = pCurItem->text();
        QString strCurPath = TcpClient::getInstance().curPath();
        m_strMoveFilePath = strCurPath + '/' + m_strMoveFileName;

        m_pSelectDirPB->setEnabled(true);
    }
    else {
        QMessageBox::warning(this, "移动文件","选择不能为空");
    }
}

void Book::selectDestDir()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(pCurItem != NULL)
    {
        QString strDestDir = pCurItem->text();
        QString strCurPath = TcpClient::getInstance().curPath();
        m_strDestDir = strCurPath + '/' + strDestDir;
        int srcLen = m_strMoveFilePath.size();
        int destLen = m_strDestDir.size();
        PDU *pdu = mkPDU(srcLen + destLen + 2);
        pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData, "%d %d %s", srcLen, destLen, m_strMoveFileName.toStdString().c_str());

        memcpy((char*)(pdu->caMsg), m_strMoveFilePath.toStdString().c_str(), srcLen);
        memcpy((char*)(pdu->caMsg) + srcLen + 1, m_strDestDir.toStdString().c_str(),destLen);

        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
    else {
        QMessageBox::warning(this, "移动文件","选择不能为空");
    }
    m_pSelectDirPB->setEnabled(false);
}
