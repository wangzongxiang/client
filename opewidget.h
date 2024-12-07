#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QWidget>
#include "book.h"
#include "friends.h"
#include <QStackedWidget>
#include <QListWidget>
#include "friends.h"

class OpeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OpeWidget(QWidget *parent = nullptr);
    QListWidget* listw;
    Friends* f;
    Book* book;
    QStackedWidget* Swidget;

signals:
};

#endif // OPEWIDGET_H
