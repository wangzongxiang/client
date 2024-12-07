#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include<QFile>
#include "OpeWidget.h"
#include "privatechat.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class tcpclient;
}
QT_END_NAMESPACE

class tcpclient : public QWidget
{
    Q_OBJECT

public:
    tcpclient(QWidget *parent = nullptr);
    ~tcpclient();
    void loadconfig();
    QTcpSocket* tcp;
    void recmsg();
    static tcpclient* getInstance();
    QTcpSocket& getTcp();
    QString name;
    QString dirpath;
    QString rootpath;

    QFile mfile;
    qint64 total;
    qint64 mrecve;
    bool download;

signals:
    void sentName(QStringList);
    void exitusr(int);
    void refreshFriend(QStringList);
private slots:

    void on_login_clicked();

    void on_register_2_clicked();

    void on_quit_clicked();//注销
private:
    Ui::tcpclient *ui;
    QString ip;
    unsigned short port;
    OpeWidget* ope;
    QList<PrivateChat*> p;



};
#endif // TCPCLIENT_H
