#include "tcpclient.h"
#include "ui_tcpclient.h"
#include "ui_privatechat.h"
#include <QMessageBox>
#include "protocol.h"
#include "opewidget.h"
#include <QStringList>
#include <QDialogButtonBox>
#include "book.h"
tcpclient::tcpclient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::tcpclient)
{
    ui->setupUi(this);
    setFixedSize(500,300);
    tcp =new QTcpSocket(this);
    loadconfig();
    tcp->connectToHost(QHostAddress(this->ip),this->port);
    connect(tcp,&QTcpSocket::connected,[=](){
        QMessageBox::critical(this,"服务器连接","服务器连接成功");
    });
    connect(tcp,&QTcpSocket::disconnected,[=]{
        QMessageBox::critical(this,"服务器连接","与服务器断开连接");
    });
    connect(tcp,&QTcpSocket::readyRead,this,&tcpclient::recmsg);
    this->download=false;
}

tcpclient* tcpclient::getInstance()
{
    static tcpclient* instance=new tcpclient();
    return instance;
}

QTcpSocket &tcpclient::getTcp()
{
    return *tcpclient::getInstance()->tcp;
}



tcpclient::~tcpclient()
{
    delete ui;
}

void tcpclient::loadconfig()//加载资源文件
{
    QFile f(":/clientplus.config");
    if(f.open(QFile::ReadOnly)){
        QByteArray arr=f.readAll();
        QString str=arr.toStdString().c_str();
        f.close();
        str.replace("\r\n"," ");
        QStringList strlist=str.split(" ");
        this->ip=strlist[0];
        this->port=strlist[1].toUShort();
    }else{
        QMessageBox::critical(this,"警告","配置文件加载失败");
    }
}

void tcpclient::recmsg()
{
    if(!tcpclient::getInstance()->download){
        unsigned int pdulen=0;
        tcp->read((char*)&pdulen,sizeof(unsigned int));
        PDU* pdu=mkPDU(pdulen-sizeof(PDU));
        tcp->read((char*)pdu+sizeof(unsigned int),pdulen-sizeof(unsigned int));
        switch (pdu->msgType) {
        case MSG_TYPE_REGIST_RESPOND:
        {
            if(strcmp(pdu->cData,msg_ok)==0) QMessageBox::information(this,"注册","您已经注册成功");
            else QMessageBox::information(this,"注册","用户名已经被使用，请使用其他用户名");
        }
        break;
        case MSG_TYPE_LOGIN_RESPOND:
        {
            if(strcmp(pdu->cData,msg_ok)==0){
                QMessageBox::information(this,"登录","成功登录");
                tcpclient::getInstance()->name=ui->name->text();
                tcpclient::getInstance()->ope=new OpeWidget();
                tcpclient::getInstance()->ope->show();
                this->close();
                qDebug()<<tcpclient::getInstance()->name;
                tcpclient::getInstance()->dirpath=QString("./"+tcpclient::getInstance()->name);//当前目录位置
                tcpclient::getInstance()->rootpath=QString("./"+tcpclient::getInstance()->name);//根目录位置
                qDebug()<<this->dirpath;
            }
            else QMessageBox::information(this,"登录","请输入正确的用户名和密码");
        }
        break;
        case MSG_TYPE_DEL_RESPOND:
        {
            if(strcmp(pdu->cData,msg_ok)==0) QMessageBox::information(this,"注销","您已经注销成功");
            else QMessageBox::information(this,"注销","请输入正确的用户名和密码来注销账户");
        }
        break;
        case MSG_TYPE_ONLINE_RESPOND:
        {
            QStringList strlist;
            int n=pdu->msglen/32;//姓名的个数
            char temp[32];
            for(int i=0;i<n;i++){
                memcpy(temp,(char*)pdu->msg+(i*32),32);
                strlist.append(QString(temp));
            }
            emit this->sentName(strlist);
        }
        break;
        case MSG_TYPE_SEARCHUSR_RESPOND:
        {
            int a;
            memcpy((char*)&a,pdu->cData,sizeof(int));
            emit this->exitusr(a);
        }
        break;
        case MSG_TYPE_FRIEND_REQUEST://别人发送好友请求
        {
            char name1[32]={"\n"};
            memcpy(name1,pdu->cData+32,32);
            qDebug()<<"发送到对方，请求添加好友"+QString(name1);
            int t=QMessageBox::information(NULL,"添加好友",QString(name1)+"请求添加你为好友",QDialogButtonBox::Yes,QDialogButtonBox::No);
            PDU* pdus=mkPDU(0);
            qDebug()<<"已同意请求，name:"<<tcpclient::getInstance()->name;
            memcpy(pdus->cData,pdu->cData,64);
            if(t==QDialogButtonBox::Yes){//接受好友请求
                pdus->msgType=MSG_TYPE_AGREE_FRIEND_RESPOND;
            }else{//拒绝好友请求
                pdus->msgType=MSG_TYPE_DISAGREE_FRIEND_RESPOND;
            }
            tcpclient::getInstance()->getTcp().write((char*)pdus,pdus->PDUlen);
            free(pdus);
            pdus=NULL;
        }
        break;
        case MSG_TYPE_FRIEND_RESPOND://给别人发送好友请求后的回应
        {
            int a;
            memcpy((char*)&a,pdu->cData,sizeof(int));
            qDebug()<<"接收到的情况为："<<a;
            if(a==0){
                QMessageBox::warning(this,"添加好友","该用户不在线，不能添加");
            }else if(a==-1){
                QMessageBox::warning(this,"添加好友","添加失败，你们已经是好友了");
            }else if(a==1){
                QMessageBox::information(this,"添加好友","添加成功，你们已经是好友了");
            }else if(a==3){
                QMessageBox::warning(this,"添加好友","不能添加自己为好友");
            }else{
                QMessageBox::warning(this,"添加好友","对方拒绝了您的好友请求");
            }
        }
        break;
        case MSG_TYPE_REFREASH_RESPOND://刷新在线好友
        {
            qDebug()<<"已接收到在线好友列表";
            QStringList str1;
            int len=pdu->msglen/32;
            char temp[32]={"\n"};
            for(int i=0;i<len;i++){
                memcpy(temp,(char*)pdu->msg+i*32,32);
                qDebug()<<QString(temp);
                str1.append(QString(temp));
            }
            emit this->refreshFriend(str1);
        }
        break;
        case MSG_TYPE_DELFRIEND_REPOND://刷新在线好友
        {
            QMessageBox::information(NULL,"删除好友","删除好友成功");
        }
        break;
        case MSG_TYPE_PRIVATECHAT_REQUEST://私聊请求
        {
            qDebug()<<"已进入到对方客户端";
            char name1[32]={"\n"};
            memcpy(name1,pdu->cData,32);
            int t=QMessageBox::information(NULL,"聊天请求",QString(name1)+"请求聊天",QDialogButtonBox::Yes,QDialogButtonBox::No);
            if(t==QDialogButtonBox::Yes){
                PrivateChat* p1=new PrivateChat();
                p1->DName=QString(name1);
                tcpclient::getInstance()->p.append(p1);
                p1->show();
                pdu->msgType=MSG_TYPE_AGREE_PRIVATECHAT_RESPOND;
            }else{
                pdu->msgType=MSG_TYPE_DISAGREE_PRIVATECHAT_RESPOND;
            }
            tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
        }
        break;
        case MSG_TYPE_AGREE_PRIVATECHAT_RESPOND://接受聊天请求
        {
            qDebug()<<"已接收到同意聊天响应";
            char name[32]={"\n"};
            memcpy(name,pdu->cData+32,32);
            PrivateChat* p1=new PrivateChat();
            p1->DName=QString(name);
            tcpclient::getInstance()->p.append(p1);
            p1->show();
        }
        break;
        case MSG_TYPE_DISAGREE_PRIVATECHAT_RESPOND://拒绝聊天请求
        {
            QMessageBox::information(NULL,"聊天请求响应","对方拒绝了你的聊天请求");
        }
        break;
        case MSG_TYPE_CHAT:
        {
            char str[]={"\n"};
            memcpy(str,(char*)pdu->msg,pdu->msglen);
            QString s=QString(str);
            char name[32]={"\n"};
            memcpy(name,pdu->cData+32,32);
            for(int i=0;i<tcpclient::getInstance()->p.size();i++){
                if(tcpclient::getInstance()->p[i]->DName==name){
                    tcpclient::getInstance()->p[i]->ui->textBrowser->append(QString(name)+":");
                    tcpclient::getInstance()->p[i]->ui->textBrowser->append(s);
                    break;
                }
            }
        }
        break;
        case MSG_TYPE_CREATE_DIR_RESPOND://创建文件夹响应
        {
            int k;
            memcpy((char*)&k,pdu->cData,sizeof(int));
            if(k)
                QMessageBox::information(NULL,"创建文件夹","文件夹创建成功");
            else
                QMessageBox::warning(NULL,"创建文件夹","文件夹创建失败，已存在该名字的文件夹");
        }
        break;
        case MSG_TYPE_FLUSH_FILE_RESPOND:
        {
            qDebug()<<"客户端接收到pdu";
            tcpclient::getInstance()->ope->book->flushdir(pdu);
            qDebug()<<"走完了";
            free(pdu);
            pdu=NULL;
        }
        break;
        case MSG_TYPE_DEL_DIR_RESPOND:
        {
            bool t;
            memcpy((char*)&t,pdu->cData,sizeof(bool));
            if(t){
                QMessageBox::information(NULL,"删除文件夹","文件夹删除成功");
            }else{
                QMessageBox::warning(NULL,"删除文件夹","文件夹删除失败");
            }
        }
        break;
        case MSG_TYPE_RENAME_DIR_RESPOND:
        {
            qDebug()<<"重命名响应";
            bool t;
            memcpy((char*)&t,pdu->cData,sizeof(bool));
            if(t){
                QMessageBox::information(NULL,"重命名文件夹","重命名文件夹成功");
            }else{
                QMessageBox::information(NULL,"重命名文件夹","重命名文件夹失败");
            }
        }
        break;
        case MSG_TYPE_ENTER_DIR_RESPOND:
        {
            int k;
            memcpy((char*)&k,pdu->cData+32,sizeof(int));
            if(k==0){
                QMessageBox::warning(NULL,"进入下一级","不是文件夹项目无法进入");
            }else{
                char name[32]={'\0'};
                memcpy(name,pdu->cData,32);
                qDebug()<<QString(name);
                tcpclient::getInstance()->dirpath=tcpclient::getInstance()->dirpath+"/"+QString(name);
                tcpclient::getInstance()->ope->book->flushdir(pdu);
                free(pdu);
                pdu=NULL;
            }
        }
        break;
        case MSG_TYPE_RETURN_DIR_RESPOND:
        {
            qDebug()<<"接收到返回上一级响应";
            QString path=tcpclient::getInstance()->dirpath;
            int index=path.lastIndexOf('/');
            tcpclient::getInstance()->dirpath=path.remove(index, path.size() - index);
            tcpclient::getInstance()->ope->book->flushdir(pdu);
            free(pdu);
            pdu=NULL;
        }
        break;
        case MSG_TYPE_UPLOAD_FINISH_RESPOND:
        {
            QMessageBox::information(NULL,"上传文件","文件上传成功");
        }
        break;
        case MSG_TYPE_DEL_FILE_RESPOND:
        {
            bool t;
            memcpy((char*)&t,pdu->cData,sizeof(bool));
            if(t){
                QMessageBox::information(NULL,"删除文件","删除文件成功");
            }else{
                QMessageBox::warning(NULL,"删除文件","删除文件失败,重新选择文件");
            }
        }
        break;
        case MSG_TYPE_DOWNLOAD_FILE_RESPOND:
        {
            memcpy((char*)&tcpclient::getInstance()->total,pdu->cData,sizeof(qint64));
            tcpclient::getInstance()->download=true;
            qDebug()<<"总共的长度为："<<tcpclient::getInstance()->total;
        }
        break;
        default:
            break;
        }
    }else{
        QByteArray arr=tcpclient::getInstance()->getTcp().readAll();
        tcpclient::getInstance()->mfile.write(arr);
        tcpclient::getInstance()->mrecve+=+arr.size();
        if(tcpclient::getInstance()->total==tcpclient::getInstance()->mrecve){
            tcpclient::getInstance()->mfile.close();
            this->download=false;
            QMessageBox::information(NULL,"下载文件","文件下载成功");
        }
    }
}

void tcpclient::on_login_clicked()
{
    QString name=ui->name->text();
    QString pwd=ui->pwd->text();
    if(name!=""&&pwd!=""){
        PDU* pdu=mkPDU(0);
        pdu->msgType=MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->cData,name.toStdString().c_str(),32);
        strncpy(pdu->cData+32,pwd.toStdString().c_str(),32);
        tcp->write((char*)pdu,pdu->PDUlen);
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::warning(this,"登录","登录的用户名或密码不能为空");
    }
}


void tcpclient::on_register_2_clicked()
{
    QString name=ui->name->text();
    QString pwd=ui->pwd->text();
    if(!name.isEmpty()&&!pwd.isEmpty()){//均不为空
        PDU* pdu=mkPDU(0);
        pdu->msgType=MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->cData,name.toStdString().c_str(),32);//前32位放用户名
        strncpy(pdu->cData+32,pwd.toStdString().c_str(),32);//后32位放密码
        tcp->write((char*)pdu,pdu->PDUlen);
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::critical(this,"注册","注册的用户名和密码不能为空");
    }
}


void tcpclient::on_quit_clicked()
{
    QString name=ui->name->text();
    QString pwd=ui->pwd->text();
    if(name!=""&&pwd!=""){
        PDU* pdu=mkPDU(0);
        pdu->msgType=MSG_TYPE_DEL_REQUEST;
        strncpy(pdu->cData,name.toStdString().c_str(),32);
        strncpy(pdu->cData+32,pwd.toStdString().c_str(),32);
        tcp->write((char*)pdu,pdu->PDUlen);
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::warning(this,"注销","请正确输入注销用户的用户名和密码");
    }

}

