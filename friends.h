#ifndef FRIENDS_H
#define FRIENDS_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "online.h"
#include "protocol.h"
class Friends : public QWidget
{
    Q_OBJECT
public:
    explicit Friends(QWidget *parent = nullptr);
    QTextEdit* pshowmsg;
    QListWidget* m_friendListWidget;
    QLineEdit* inputMsgle;

    QPushButton* DelFriendPB;
    QPushButton* FlushFriendPB;
    QPushButton* ShowOnlineUsrPB;
    QPushButton* SearchUsrPB;
    QPushButton* MsgSendPB;
    QPushButton* PrivateChatPB;
    Online* online1;

signals:
};

#endif // FRIENDS_H
