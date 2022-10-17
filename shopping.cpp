#include "shopping.h"
#include "product.h"

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
    return text(0).toInt();
}

QString Shopping::getClientName()
{
    return text(1);
}

void Shopping::setClientName(QString& Client)
{
    setText(1, Client);
}

QString Shopping::getProductName()
{
    return text(2);
}

void Shopping::setProductName(QString& Product)
{
    setText(2, Product);
}

QString Shopping::getDate()
{
    return text(3);
}

void Shopping::setDate(QString& date)
{
    setText(3, date);
}

int Shopping::getquan()
{
    return text(4).toInt();
}

void Shopping::setquan(int& quan)
{
    setText(4, QString::number(quan));
}

int Shopping::getAllPrice()
{
    return text(5).toInt();
}

void Shopping::setAllPrice(int& price)
{
    setText(5, QString::number(price));
}


