#include "product.h"

Product::Product(int id, QString name, QString company, int price)
{
    setText(0, QString::number(id));
    setText(1, name);
    setText(2, company);
    setText(3, QString::number(price));
}

int Product::getid()
{
    return text(0).toInt();
}

QString Product::getName()
{
    return text(1);
}
void Product::setName(QString& name)
{
    setText(1, name);
}

QString Product::getCompany()
{
    return text(2);
}

void Product::setCompany(QString& company)
{
    setText(2, company);
}

int Product::getPrice()
{
    return text(3).toInt();
}

void Product::setPrice(int& price)
{
    setText(3, QString::number(price));
}

bool Product::operator==(const Product& other)const
{
    return (this->text(1) == other.text(1));
}
