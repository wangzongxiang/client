#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include "protocol.h"
#include <QFile>
class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void flushdir(PDU* pdu);
    void uploadFiledata();
private:
    QListWidget* boollist;
    QPushButton* returnPB;
    QPushButton* createDirPB;
    QPushButton* delDirPB;//删除文件夹
    QPushButton* renameDirPB;
    QPushButton* refreshPB;
    QPushButton* uploadPB;
    QPushButton* delfilePB;//删除文件
    QPushButton* dowmloadPB;
    QPushButton* sharePB;

    QString uploadpath;//上传文件路径
    QTimer* mtimer;//定时器，防止上传文件时粘包



signals:
};

#endif // BOOK_H
