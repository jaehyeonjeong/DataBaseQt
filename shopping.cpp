#include "shopping.h"

        /*QTreeWidgetItem을 상속받았기 때문에 setText(column, text)형태로 데이터를 저장*/
Shopping::Shopping(int id, QString Client, QString Product, QString Date, int quan, int AllPrice)
{
    setText(0, QString::number(id));
    setText(1, Client);
    setText(2, Product);
    setText(3, Date);
    setText(4, QString::number(quan));
    setText(5, QString::number(AllPrice));
}

int Shopping::getId()
{
    return text(0).toInt();     /*treeWidget column 1*/
}

QString Shopping::getClientName()
{
    return text(1);             /*treeWidget column 2*/
}

void Shopping::setClientName(QString& Client)
{
    setText(1, Client);
}

QString Shopping::getProductName()
{
    return text(2);             /*treeWidget column 3*/
}

void Shopping::setProductName(QString& Product)
{
    setText(2, Product);
}

QString Shopping::getDate()
{
    return text(3);             /*treeWidget column 4*/
}

void Shopping::setDate(QString& date)
{
    setText(3, date);
}

int Shopping::getquan()
{
    return text(4).toInt();     /*treeWidget column 5*/
}

void Shopping::setquan(int& quan)
{
    setText(4, QString::number(quan));
}

int Shopping::getAllPrice()
{
    return text(5).toInt();     /*treeWidget column 6*/
}

void Shopping::setAllPrice(int& price)
{
    setText(5, QString::number(price));
}


