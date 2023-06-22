#include "sharefile.h"
#include <QDebug>
#include "tcpclient.h"
#include "opewidget.h"
shareFile::shareFile(QWidget *parent) : QWidget(parent)
{
    m_pSelectAllPB = new QPushButton("全选");
    m_pCanCelSelectPB = new QPushButton("取消选择");

    m_pOKPB = new QPushButton("确定");
    m_pCancelPB = new QPushButton("取消");

    m_pSA = new QScrollArea;
    m_pFriendW = new QWidget;
    m_pFirendWVBL = new QVBoxLayout(m_pFriendW);
    m_pButtonGroup = new QButtonGroup(m_pFriendW);
    m_pButtonGroup->setExclusive(false);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCanCelSelectPB);
    pTopHBL->addStretch();

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pOKPB);
    pTopHBL->addWidget(m_pCancelPB);

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);

    setLayout(pMainVBL);

    connect(m_pCanCelSelectPB, SIGNAL(clicked(bool))
            , this , SLOT(cancelSelect()));
    connect(m_pSelectAllPB, SIGNAL(clicked(bool))
            , this , SLOT(selectAll()));
    connect(m_pOKPB, SIGNAL(clicked(bool))
            , this , SLOT(okShare()));
    connect(m_pCancelPB, SIGNAL(clicked(bool))
            , this , SLOT(cancelShare()));
}

shareFile &shareFile::getInstance()
{
    static shareFile instance;
    return instance;
}

void shareFile::updateFriend(QListWidget *pFriendList)
{
    qDebug()<<"客户端显示在线好友";
    if(pFriendList == NULL)
    {
        return  ;
    }
    QAbstractButton *tmp = NULL;
    QList<QAbstractButton*> preFriendList =  m_pButtonGroup->buttons();
        qDebug()<<preFriendList.size();
    for(int i = 0; i< preFriendList.size(); i++)
    {
        tmp = preFriendList[i];
        m_pFirendWVBL->removeWidget(tmp);
        m_pButtonGroup->removeButton(tmp);
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp = NULL;
    }
    qDebug()<<"客户清除原列表内容";
//    m_pButtonGroup->buttons().clear();
    QCheckBox *pCB = NULL;
    for(int i = 0; i<pFriendList->count(); i++)
    {
        pCB = new QCheckBox(pFriendList->item(i)->text());
        qDebug()<<pFriendList->item(i)->text();
        m_pFirendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
    qDebug()<<"客户端列表更新完毕";
}

void shareFile::cancelSelect()
{
    QList <QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i = 0; i < cbList.size(); i++)
    {
        if(cbList[i]->isChecked())
                cbList[i]->setChecked(false);
    }
}

void shareFile::selectAll()
{
    QList <QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i = 0; i < cbList.size(); i++)
    {
        if(!cbList[i]->isChecked())
                cbList[i]->setChecked(true);
    }
}

void shareFile::okShare()
{
    qDebug()<<"确认按钮点击";
    QString strName = TcpClient::getInstance().loginName();
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strShareFileName = OpeWidget::getInstance().getBook()->getShareFileName();
    QList <QAbstractButton*> cbList = m_pButtonGroup->buttons();
    QString strPath = strCurPath + "/" + strShareFileName;
    int num = 0;
    for(int i = 0; i < cbList.size(); i++)
    {
        if(cbList[i]->isChecked())
            num++;
    }
    PDU *pdu = mkPDU(32*num + strPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData, "%s %d", strName.toStdString().c_str(),num);
    int j = 0;
    for(int i =0; i<cbList.size(); i++)
    {
        if(cbList[i]->isChecked())
        {
            memcpy((char*)(pdu->caMsg)+j*32,cbList[i]->text().toStdString().c_str(),cbList[i]->text().size());
            j++;
            qDebug()<<"被共享者名字：" + cbList[i]->text();
        }
    }
    qDebug()<<"共享者名字：" + strName<<"被共享者人数" << num;
    memcpy((char*)(pdu->caMsg)+num*32,strPath.toStdString().c_str(), strPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
}

void shareFile::cancelShare()
{
    hide();
}
