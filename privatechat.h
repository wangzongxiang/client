#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = nullptr);
    ~PrivateChat();
    QString MyName;//自己的名字
    QString DName;//对方的名字
private slots:
    void on_sendtextPB_clicked();

public:
    Ui::PrivateChat *ui;
};

#endif // PRIVATECHAT_H
