#include "friends.h"
#include "tcpclient.h"
#include <QInputDialog>
#include "QMessageBox"
Friends::Friends(QWidget *parent)
    : QWidget{parent}
{
    pshowmsg=new QTextEdit(this);
    m_friendListWidget=new QListWidget(this);
    inputMsgle=new QLineEdit(this);

    DelFriendPB=new QPushButton("删除好友",this);
    FlushFriendPB=new QPushButton("刷新用户",this);
    ShowOnlineUsrPB=new QPushButton("显示在线好友",this);
    SearchUsrPB=new QPushButton("查找用户",this);
    MsgSendPB=new QPushButton("发送信息",this);
    PrivateChatPB=new QPushButton("私聊",this);

    QVBoxLayout* pleftPBVBL=new QVBoxLayout;//垂直布局
    pleftPBVBL->addWidget(DelFriendPB);
    pleftPBVBL->addWidget(FlushFriendPB);
    pleftPBVBL->addWidget(ShowOnlineUsrPB);
    pleftPBVBL->addWidget(SearchUsrPB);
    pleftPBVBL->addWidget(PrivateChatPB);

    QHBoxLayout* pTop=new QHBoxLayout;
    pTop->addWidget(pshowmsg);
    pTop->addWidget(m_friendListWidget);//在线好友链表
    pTop->addLayout(pleftPBVBL);

    QHBoxLayout* phbl=new QHBoxLayout;
    phbl->addWidget(inputMsgle);
    phbl->addWidget(MsgSendPB);

    this->online1=new Online();
    connect(ShowOnlineUsrPB,&QPushButton::clicked,[=](){
        if(ShowOnlineUsrPB->text()=="显示在线好友"){
            PDU* pdu=mkPDU(0);
            pdu->msgType=MSG_TYPE_ONLINE_REQUEST;
            tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
            this->online1->show();
            ShowOnlineUsrPB->setText("关闭在线好友");
        }else{
            this->online1->hide();
            ShowOnlineUsrPB->setText("显示在线好友");
        }
    });

    connect(SearchUsrPB,&QPushButton::clicked,[=](){
        QString name=QInputDialog::getText(this,"搜索用户","请输入查找的用户名");
        PDU* pdu=mkPDU(0);
        pdu->msgType=MSG_TYPE_SEARCHUSR_REQUEST;
        memcpy(pdu->cData,name.toStdString().c_str(),32);
        tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
    });


    connect(tcpclient::getInstance(),&tcpclient::exitusr,[=](int a){
        if(a==-1){
            QMessageBox::warning(this,"查找用户","该用户不存在");
        }else if(a==0){
            QMessageBox::information(this,"查找用户","该用户暂未上线");
        }else{
            QMessageBox::information(this,"查找用户","该用户正在线上");
        }
    });
    connect(FlushFriendPB,&QPushButton::clicked,[=](){//刷新好友
        PDU* pdu=mkPDU(0);
        pdu->msgType=MSG_TYPE_REFREASH_REQUEST;
        memcpy(pdu->cData,tcpclient::getInstance()->name.toStdString().c_str(),32);
        tcpclient::getInstance()->tcp->write((char*)pdu,pdu->PDUlen);
    });
    connect(tcpclient::getInstance(),&tcpclient::refreshFriend,[=](QStringList str1){
        m_friendListWidget->clear();
        for(int i=0;i<str1.size();i++){
            qDebug()<<"获取到的在线好友为  "<<str1[i];
             m_friendListWidget->addItem(str1[i]);
        }
    });
    connect(DelFriendPB,&QPushButton::clicked,[=](){
        QString delname=m_friendListWidget->currentItem()->text();
        QMessageBox::StandardButton t=QMessageBox::information(NULL,"删除好友","确定要删除好友"+delname,QMessageBox::Yes,QMessageBox::No);
        if(t==QMessageBox::Yes){
            PDU* pdu=mkPDU(0);
            pdu->msgType=MSG_TYPE_DELFRIEND_REQUEST;
            memcpy(pdu->cData,delname.toStdString().c_str(),32);
            memcpy(pdu->cData+32,tcpclient::getInstance()->name.toStdString().c_str(),32);
            tcpclient::getInstance()->tcp->write((char*)pdu,pdu->PDUlen);
        }
    });
    connect(PrivateChatPB,&QPushButton::clicked,[=](){
        PDU* pdu=mkPDU(0);
        pdu->msgType=MSG_TYPE_PRIVATECHAT_REQUEST;
        QString myname=tcpclient::getInstance()->name;
        qDebug()<<"我的名字"<<myname;
        QString Dname=m_friendListWidget->currentItem()->text();
        qDebug()<<"对方名字"<<Dname;
        memcpy(pdu->cData,myname.toStdString().c_str(),myname.size());
        memcpy(pdu->cData+32,Dname.toStdString().c_str(),Dname.size());
        tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
    });
    QVBoxLayout* pMain=new QVBoxLayout;//垂直布局
    pMain->addLayout(pTop);
    pMain->addLayout(phbl);
    setLayout(pMain);

}
