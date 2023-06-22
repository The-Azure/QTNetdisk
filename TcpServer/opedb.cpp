#include "opedb.h"
#include <QMessageBox>
#include <QDebug>
OpeDB::OpeDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("D:\\work\\qtproject\\cloud.db");
    if(m_db.open())
    {
        qDebug() << "成功打开数据库";
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while(query.next())
        {
            QString data = QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
        }
    }
    else
    {
        QMessageBox::critical(NULL,"打开数据库","打开数据库失败");
    }
}

OpeDB::~OpeDB()
{
    m_db.close();
}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if(NULL == name || pwd == NULL)
    {
        return false;
    }

    QString data = QString("insert into usrInfo(name, pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    qDebug() << "数据库添加注册信息";
    QSqlQuery query;
    return query.exec(data);

}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if(NULL == name || pwd == NULL)
    {
        return false;
    }
    QString data = QString("select * from usrInfo where name=\'%1\' and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(data);
    if(query.next())//如果上调语句执行成功，则next会获得返回的数据并且保存到query中，并返回真
    {
        data = QString("update usrInfo set online = 1 where name=\'%1\' and pwd = \'%2\'").arg(name).arg(pwd);
        qDebug() << QString("登陆成功，用户%1在线").arg(name);
        QSqlQuery query;
        return query.exec(data);
    }
    else
        return false;
}

void OpeDB::handleOffline(const char *name)
{
    if(NULL == name)
    {
        qDebug() <<"输入名字不合法";
    }
    QString data = QString("update usrInfo set online = 0 where name=\'%1\'").arg(name);
    qDebug() << QString("服务器即将关闭，用户%1离线").arg(name);
    QSqlQuery query;
    query.exec(data);
}

QStringList OpeDB::handleAllOnline()
{
    QString data = QString("select name from usrInfo where online = 1");
    QSqlQuery query;
    query.exec(data);
    QStringList result;
    result.clear();
    qDebug() << "数据库查询在线用户";
    while(query.next())
    {
        result.append(query.value(0).toString());
    }
    return result;
}

int OpeDB::handleSearchUsr(const char *name)
{
    if(name == NULL)
    {
        return -1;
    }
    QString data = QString("select online from usrInfo where name = \'%1\'").arg(name);
    qDebug() << "数据库查询用户";
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        return query.value(0).toInt();
    }
    else{
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if(NULL == pername || NULL == name || *name == *pername)
    {
        return -1;
    }
    QString data = QString("select * from friend where id = (select id from usrInfo where name=\'%1\') and friendId = (select id from usrInfo where name = \'%2\')"
                           "or id = (select id from usrInfo where name=\'%3\') and friendId = (select id from usrInfo where name = \'%4\')").arg(name).arg(pername).arg(pername).arg(name);
    qDebug() << "数据库查询是否是好友关系";
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        return 0;//已经是好友
    }
    else
    {
        data = QString("select online from usrInfo where name = \'%1\'").arg(pername);
        qDebug() << "查询欲添加的好友是否离线";
        QSqlQuery query;
        query.exec(data);
        if(query.next())
        {
            int ret = query.value(0).toInt();
            if(ret == 1)
            {
                return 1;//在线
            }
            else
            {
                return 2;//不在线
            }
        }

        else{
            return 3;//欲添加的好友名字不存在
        }
    }
}

void OpeDB::handleAddFriendRespond(const char *pername, const char *name)
{
    QString data = QString("insert into friend values((select id from usrInfo where name=\'%1\'),(select id from usrInfo where name=\'%2\'))").arg(name).arg(pername);
    qDebug() << "数据库添加好友";
    QSqlQuery query;
    query.exec(data);
}

QStringList OpeDB::handleFlushFriend(const char *name)
{
    QStringList strFriendList;
    strFriendList.clear();
    if(name == NULL)
    {
        return strFriendList;
    }
    QString data = QString("select name from usrInfo where online = 1 and id in (select id from friend where friendId = (select id from usrInfo where name =\'%1\'))").arg(name);
    qDebug()<< data;
    qDebug() << "数据库搜索好友";
    QSqlQuery query;
    query.exec(data);
    while(query.next())
    {
        strFriendList.append(query.value(0).toString());
    }

    data = QString("select name from usrInfo where online = 1 and id in (select friendId from friend where id = (select id from usrInfo where name =\'%1\'))").arg(name);
    query.exec(data);
    while(query.next())
    {
        strFriendList.append(query.value(0).toString());
    }
    return strFriendList;
}

bool OpeDB::handleDelFriend(const char *name, const char *friendName)
{
    if(NULL == name || NULL == friendName)
        return false;
    QString data = QString("delete from friend where id=(select id from usrInfo where name = \'%1\') and friendId = (select id from usrInfo where name = \'%2\')").arg(name).arg(friendName);
    qDebug() << "数据库删除好友";
    qDebug()<< data;
    QSqlQuery query;
    query.exec(data);

    data = QString("delete from friend where id=(select id from usrInfo where name = \'%1\') and friendId = (select id from usrInfo where name = \'%2\')").arg(friendName).arg(name);
    query.exec(data);
    return true;
}
