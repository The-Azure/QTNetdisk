#ifndef UPLOADFILE_H
#define UPLOADFILE_H

#include <QThread>
#include <QTcpSocket>
#include "protocol.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QRunnable>
class uploadFile: public QObject, public QRunnable
{
    Q_OBJECT


public:
    QString getCurPath();
    explicit uploadFile();

    void run() override;

public slots:

private:
    QString filePath;
    QString curPath;
};











#endif // UPLOADFILE_H
