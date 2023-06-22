#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QWidget>
#include <QListWidget>
class OpeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OpeWidget(QWidget *parent = nullptr);

signals:

public slots:

private:
    QListWidget *m_pListW;

};

#endif // OPEWIDGET_H
