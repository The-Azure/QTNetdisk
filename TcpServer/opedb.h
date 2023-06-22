#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    static OpeDB& getInstance();//使得可以通过类函数获得同一个对象
    void init();
    ~OpeDB();

    bool handleRegist(const char *name, const char *pwd);
    bool handleLogin(const char *name, const char *pwd);

    void handleOffline(const char *name);
    QStringList handleAllOnline();
    int handleSearchUsr(const char* name);
    int handleAddFriend(const char* pername, const char *name);
    void handleAddFriendRespond(const char* pername, const char *name);
    QStringList handleFlushFriend(const char* name);
    bool handleDelFriend(const char* name, const char *friendName);

signals:

public slots:
private:
    QSqlDatabase m_db;//用于连接数据库
};

#endif // OPEDB_H
