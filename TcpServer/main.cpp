#include "tcpserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    OpeDB::getInstance().init();//数据库初始化


    TcpServer w;
    w.show();

    return a.exec();
}
