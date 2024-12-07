#include "book.h"
#include "protocol.h"
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <tcpclient.h>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
Book::Book(QWidget *parent)
    : QWidget{parent}
{
    boollist=new QListWidget;//文件夹列表
    returnPB=new QPushButton("返回");//返回
    createDirPB=new QPushButton("创建文件夹");//创建文件夹
    delDirPB=new QPushButton("删除文件夹");//删除文件夹
    renameDirPB=new QPushButton("重命名文件夹");//重命名文件夹
    refreshPB=new QPushButton("刷新文件夹");//刷新文件夹

    QVBoxLayout* pDir=new QVBoxLayout;
    pDir->addWidget(returnPB);
    pDir->addWidget(createDirPB);
    pDir->addWidget(delDirPB);
    pDir->addWidget(renameDirPB);
    pDir->addWidget(refreshPB);


    uploadPB=new QPushButton("上传文件");//上传文件
    delfilePB=new QPushButton("删除文件");//删除文件
    dowmloadPB=new QPushButton("下载文件");//下载文件
    sharePB=new QPushButton("分享文件");//分享文件

    QVBoxLayout* pfile=new QVBoxLayout;
    pfile->addWidget(uploadPB);
    pfile->addWidget(delfilePB);
    pfile->addWidget(dowmloadPB);
    pfile->addWidget(sharePB);

    QHBoxLayout* pmain=new QHBoxLayout;
    pmain->addWidget(boollist);
    pmain->addLayout(pDir);
    pmain->addLayout(pfile);
    setLayout(pmain);

    connect(createDirPB,&QPushButton::clicked,[=]{
        QString dirname=QInputDialog::getText(this,"新建文件夹","新文件夹名为(字数<32)");
        if(dirname.size()==0) {
            QMessageBox::warning(this,"创建文件夹","新建文件夹名不能为空");
            return;
        }
        QString dirpath=tcpclient::getInstance()->dirpath;
        qDebug()<<dirpath;
        PDU* pdu=mkPDU(dirpath.size()+1);
        pdu->msgType=MSG_TYPE_CREATE_DIR_REQUEST;
        memcpy(pdu->cData,tcpclient::getInstance()->name.toStdString().c_str(),tcpclient::getInstance()->name.size());
        memcpy(pdu->cData+32,dirname.toStdString().c_str(),dirname.size());
        memcpy((char*)pdu->msg,dirpath.toStdString().c_str(),dirpath.size());
        tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
        free(pdu);
        pdu=NULL;
    });
    connect(refreshPB,&QPushButton::clicked,[=](){
        QString curpath=tcpclient::getInstance()->dirpath;
        qDebug()<<curpath;
        PDU* pdu=mkPDU(curpath.size()+1);
        pdu->msgType=MSG_TYPE_FLUSH_FILE_REQUEST;
        memcpy((char*)pdu->msg,curpath.toStdString().c_str(),curpath.size());
        tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
    });
    connect(delDirPB,&QPushButton::clicked,[=](){
        QString curpath=tcpclient::getInstance()->dirpath;
        QListWidgetItem* pitem=boollist->currentItem();
        if(pitem==NULL){
            QMessageBox::warning(NULL,"删除文件","请选择要删除的文件");
        }else{
            QString delname=pitem->text();
            PDU* pdu=mkPDU(curpath.size()+1);
            qDebug()<<curpath<<"  "<<delname;
            pdu->msgType=MSG_TYPE_DEL_DIR_REQUEST;
            memcpy(pdu->cData,delname.toStdString().c_str(),delname.size());
            memcpy((char*)pdu->msg,curpath.toStdString().c_str(),curpath.size());
            tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
            free(pdu);
            pdu=NULL;
        }
    });
    connect(renameDirPB,&QPushButton::clicked,[=](){
        QListWidgetItem* pitem=boollist->currentItem();
        if(pitem==NULL) {
            QMessageBox::warning(NULL,"重命名文件夹","请选择文件夹来重命名");

        }else{
            QString name=QInputDialog::getText(NULL,"文件夹重命名","重命名为：");
            QString curname=pitem->text();
                if(name.size()==0) QMessageBox::warning(NULL,"重命名文件夹","文件夹名不能为空");
            else{
                QString curpath=tcpclient::getInstance()->dirpath;
                PDU* pdu=mkPDU(curpath.size()+1);
                pdu->msgType=MSG_TYPE_RENAME_DIR_REQUEST;
                qDebug()<<curname<<"   "<<name;
                memcpy(pdu->cData,name.toStdString().c_str(),name.size());//新文件夹名
                memcpy(pdu->cData+32,curname.toStdString().c_str(),curname.size());//旧文件夹名
                memcpy(pdu->msg,curpath.toStdString().c_str(),curpath.size());
                tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
            }
        }
    });
    connect(boollist,&QListWidget::doubleClicked,[=](QModelIndex p){
        QString name=p.data().toString();
        QString curpath=tcpclient::getInstance()->dirpath;
        PDU* pdu=mkPDU(curpath.size()+1);
        pdu->msgType=MSG_TYPE_ENTER_DIR_REQUEST;
        memcpy(pdu->cData,name.toStdString().c_str(),name.size());
        memcpy((char*)pdu->msg,curpath.toStdString().c_str(),curpath.size());
        tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
        free(pdu);
        pdu=NULL;
    });
    connect(returnPB,&QPushButton::clicked,[=](){
        QString curpath=tcpclient::getInstance()->dirpath;
        if(curpath==tcpclient::getInstance()->rootpath){
            QMessageBox::warning(NULL,"上一级","已经到最到达根目录，无法向上返回");
        }else{
            int index = curpath.lastIndexOf("/");
            curpath = curpath.remove(index, curpath.size() - index);
            qDebug()<<curpath;
            PDU* pdu=mkPDU(curpath.size()+1);
            pdu->msgType=MSG_TYPE_RETURN_DIR_REQUEST;
            memcpy((char*)pdu->msg,curpath.toStdString().c_str(),curpath.size());
            tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
            free(pdu);
            pdu=NULL;
        }
    });
    this->mtimer=new QTimer;//上传定时器
    connect(this->mtimer,&QTimer::timeout,this,&Book::uploadFiledata);
    connect(uploadPB,&QPushButton::clicked,[=](){
        QString curpath=tcpclient::getInstance()->dirpath;
        this->uploadpath=QFileDialog::getOpenFileName();
        if(this->uploadpath.size()==0){
            QMessageBox::warning(NULL,"上传文件","请选择上传文件");
        }else{
            int index=this->uploadpath.lastIndexOf('/');
            qDebug()<<this->uploadpath;
            QString path=this->uploadpath;
            QString filename=this->uploadpath.remove(0,index-0+1);
            this->uploadpath=path;
            qDebug()<<filename;
            QFile file(this->uploadpath);
            qint64 filesize=file.size();
            PDU* pdu=mkPDU(curpath.size()+1);
            pdu->msgType=MSG_TYPE_UPLOAD_FILE_REQUEST;
            memcpy(pdu->cData,filename.toStdString().c_str(),filename.size());
            memcpy(pdu->msg,curpath.toStdString().c_str(),curpath.size());
            memcpy(pdu->cData+32,(char*)&filesize,sizeof(qint64));
            tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
            this->mtimer->start(1000);
        }
    });
    connect(delfilePB,&QPushButton::clicked,[=](){
        QString path=tcpclient::getInstance()->dirpath;
        QString name=this->boollist->currentItem()->text();
        PDU* pdu=mkPDU(path.size()+1);
        pdu->msgType=MSG_TYPE_DEL_FILE_REQUEST;
        memcpy((char*)pdu->msg,path.toStdString().c_str(),path.size());
        memcpy((char*)pdu->cData,name.toStdString().c_str(),name.size());
        tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
    });
    connect(dowmloadPB,&QPushButton::clicked,[=](){//下载文件
        QString path=tcpclient::getInstance()->dirpath;
        QString name=this->boollist->currentItem()->text();
        if(name.size()==0){
            QMessageBox::warning(NULL,"下载文件","请选择下载文件");
            return;
        }
        QString finalpath=QFileDialog::getSaveFileName();
        int index=name.lastIndexOf('.');
        PDU* pdu=mkPDU(path.size()+1);
        memcpy(pdu->msg,path.toStdString().c_str(),path.size());
        memcpy(pdu->cData,name.toStdString().c_str(),name.size());
        pdu->msgType=MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        QString lastpoint=name.remove(0,index-0);
        finalpath=finalpath+lastpoint;//下载路径
        qDebug()<<"下载路径"<<finalpath;
        tcpclient::getInstance()->mfile.setFileName(finalpath);
        tcpclient::getInstance()->mfile.open(QIODevice::WriteOnly);
        tcpclient::getInstance()->getTcp().write((char*)pdu,pdu->PDUlen);
    });
}

void Book::flushdir(PDU *pdu)
{
    qDebug()<<"进入到最终函数";
    int len=pdu->msglen/sizeof(MFileInfo);
    qDebug()<<"接收到文件个数为"<<len;
    this->boollist->clear();
    for(int i=2;i<len;i++){
        MFileInfo* p=(MFileInfo*)pdu->msg+i;
        if(strcmp(p->caName,".")==0||strcmp(p->caName,"..")==0) continue;//根目录不显示
        QListWidgetItem* pitem=new QListWidgetItem;
        if(p->FileType==0)
        pitem->setIcon(QIcon(QPixmap(":/dir.jpg")));
        else
        pitem->setIcon(QIcon(QPixmap(":/file.jpg")));
        pitem->setText(p->caName);
        qDebug()<<QString(p->caName);
        this->boollist->addItem(pitem);
    }
}

void Book::uploadFiledata()
{
    this->mtimer->stop();
    qDebug()<<"绝对路径  "<<this->uploadpath;
    QFile file(this->uploadpath);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(NULL,"文件上传","文件上传失败, 无法打开文件");
        return;
    }
    char* pBuffer=new char[4096];
    qint64 ret=0;
    while(true){
        ret=file.read(pBuffer,4096);
        if(ret>0&&ret<=4096){
            tcpclient::getInstance()->getTcp().write(pBuffer,ret);
        }else if(ret==0){
            break;
        }else{
            QMessageBox::warning(NULL,"上传文件","文件上传失败：读文件时出错");
        }
    }
    file.close();
    this->uploadpath.clear();
    delete []pBuffer;
    pBuffer=NULL;
}
