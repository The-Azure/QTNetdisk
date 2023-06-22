#include "online.h"
#include "ui_online.h"
#include <QDebug>
#include "tcpclient.h"
#include <QMessageBox>
Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUsr(PDU *pdu)
{
    if(pdu == NULL)
    {
        return;
    }
    ui->online_lw->clear();
    uint uSize = pdu->uiMsgLen/32;
    char caTmp[32];
    for(uint i = 0; i<uSize; i++)
    {
        memcpy(caTmp, (char*)(pdu->caMsg) + i * 32, 32);
        qDebug() << "收到了信息："<<(char*)(pdu->caMsg);
        ui->online_lw->addItem(caTmp);
    }
}

void Online::on_addFriend_pb_clicked()
{
    if(ui->online_lw->currentItem() != NULL)
    {
        QListWidgetItem *pItem = ui->online_lw->currentItem();
        QString strPerUsrName = pItem->text();
        QString strLoginName = TcpClient::getInstance().loginName();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
        memcpy(pdu->caData, strPerUsrName.toStdString().c_str(), strPerUsrName.size());
        memcpy(pdu->caData + 32, strLoginName.toStdString().c_str(), strLoginName.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else{
        QMessageBox::warning(this, "添加好友", "请选择想要添加的好友");
    }
}
