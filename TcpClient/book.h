#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "protocol.h"
#include <QTimer>
#include "sharefile.h"
#include "uploadFile.h"
class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(const PDU *pdu);
    QString getEnterDir();
    void clearEnterDir();
    void setDownloadStatu(bool status);
    bool getDownloadStatu();
    QString getSaveFilePath();
    QString getShareFileName();

    qint64 m_iTotal;//总的文件大小
    qint64 m_iReced;//已经收到的大小
signals:

public slots:
    void createDir();
    void flushFile();
    void delDir();
    void renameFile();
    void enterDir(const QModelIndex &index);
    void returnPre();
    void uploadfile();
    void uploadFileDate();
    void delRegFile();
    void downloadFile();
    void ShareFile();
    void moveFile();
    void selectDestDir();

private:
    QListWidget *m_pBookListW;
    QPushButton *m_pPReturnPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pPUploadPB;
    QPushButton *m_pDownLoadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pPShareFilePB;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pSelectDirPB;

    QString m_strEnterDir;
    QString m_strUploadFilePath;

    QTimer *m_pTimer;
    QString m_strSaveFilePath;

    bool m_bDownload;

    QString m_strShareFileName;
    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strDestDir;
};

#endif // BOOK_H
