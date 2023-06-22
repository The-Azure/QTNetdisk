#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QListWidget>
#include <QCheckBox>

class shareFile : public QWidget
{
    Q_OBJECT
public:
    explicit shareFile(QWidget *parent = nullptr);

    static shareFile&getInstance();

    void updateFriend(QListWidget *pFriendList);

signals:

public slots:
    void cancelSelect();
    void selectAll();
    void okShare();
    void cancelShare();

private:
    QPushButton *m_pSelectAllPB;
    QPushButton *m_pCanCelSelectPB;

    QPushButton *m_pOKPB;
    QPushButton *m_pCancelPB;

    QScrollArea *m_pSA;
    QWidget *m_pFriendW;
    QVBoxLayout *m_pFirendWVBL;
    QButtonGroup *m_pButtonGroup;
};

#endif // SHAREFILE_H
