#include "opewidget.h"

OpeWidget::OpeWidget(QWidget *parent)
    : QWidget{parent}
{
    listw=new QListWidget();
    listw->addItem("好友");
    listw->addItem("文件管理");
    this->book=new Book();
    this->f=new Friends();
    this->Swidget=new QStackedWidget();
    this->Swidget->addWidget(this->f);
    this->Swidget->addWidget(this->book);
    QHBoxLayout* hbl=new QHBoxLayout();
    hbl->addWidget(listw);
    hbl->addWidget(this->Swidget);

    setLayout(hbl);
    connect(listw,&QListWidget::currentRowChanged,this->Swidget,&QStackedWidget::setCurrentIndex);
}
