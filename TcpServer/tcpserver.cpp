#include "tcpserver.h"
#include "ui_tcpserver.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);


    MyTcpServer *fileServer = new MyTcpServer;
    fileServer->listen(QHostAddress(("127.0.0.1")), 8848);
//    sockt = new QTcpSocket(this);
//    connect(fileServer,&QTcpServer::newConnection,this,[=](){
//        sockt = fileServer->nextPendingConnection();

//        QString ip = sockt->peerAddress().toString();
//        quint16 port = sockt->peerPort();

//        qDebug()<<QString("[on_connect_f()==>%1:%2]成功连接").arg(ip).arg(port);
////        sockt->disconnect();

//        connect(sockt, &QTcpSocket::readyRead, this, [=](){
//            qDebug()<<"有文件发送过来";
//        });
//        qDebug()<<"111";
//    });
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();
        file.close();

        strData.replace("\r\n"," ");
        QStringList strList = strData.split(" ");

        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
        qDebug() << "IP"<< m_strIP << "port"<< m_usPort;
    }
    else
    {
        QMessageBox::critical(this, "open config", "open config failed");
    }
}


