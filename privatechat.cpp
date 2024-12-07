#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h"
#include "tcpclient.h"
PrivateChat::PrivateChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

void PrivateChat::on_sendtextPB_clicked()//发送
{
    QString str=ui->sendtext->toPlainText();
    PDU* pdu=mkPDU(str.size()+1);
    pdu->msgType=MSG_TYPE_CHAT;
    memcpy(pdu->cData,this->DName.toStdString().c_str(),32);
    memcpy(pdu->cData+32,tcpclient::getInstance()->name.toStdString().c_str(),32);
    memcpy((char*)pdu->msg,str.toStdString().c_str(),str.size());
    ui->textBrowser->append(tcpclient::getInstance()->name+":");
    ui->textBrowser->append(str);
    ui->sendtext->clear();
    tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
    free(pdu);
    pdu=NULL;
}

