#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"

Online::Online(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Online)
{
    ui->setupUi(this);
    connect(tcpclient::getInstance(),&tcpclient::sentName,[=](QStringList s){
        ui->listWidget->clear();
        for(int i=0;i<s.size();i++){
            qDebug()<<s[i];
            ui->listWidget->addItem(s[i]);
        }
    });
    connect(ui->addname,&QPushButton::clicked,[=](){
       QString name=ui->listWidget->currentItem()->text();
        QString name1=tcpclient::getInstance()->name;
       qDebug()<<"按钮："<<name<<" "<<name1;
        PDU* pdu=mkPDU(0);
       memcpy(pdu->cData,name.toStdString().c_str(),name.size());
        memcpy(pdu->cData+32,name1.toStdString().c_str(),name1.size());
        pdu->msgType=MSG_TYPE_FRIEND_REQUEST;
       tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
        free(pdu);
       pdu=NULL;
    });

}

Online::~Online()
{
    delete ui;
}
