#include "product.h"

/*QTreeWidgetItem을 상속받았기 때문에 setText(column, text)형태로 데이터를 저장*/
Product::Product(int id, QString name, QString company, int price)
{
    setText(0, QString::number(id));
    setText(1, name);
    setText(2, company);
    setText(3, QString::number(price));
}

int Product::getid()
{
    return text(0).toInt();     /*treeWidget column 1*/
}

QString Product::getName()
{
    return text(1);              /*treeWidget column 2*/
}
void Product::setName(QString& name)
{
    setText(1, name);
}

QString Product::getCompany()
{
    return text(2);             /*treeWidget column 3*/
}

void Product::setCompany(QString& company)
{
    setText(2, company);
}

int Product::getPrice()
{
    return text(3).toInt();      /*treeWidget column 4*/
}

void Product::setPrice(int& price)
{
    setText(3, QString::number(price));
}

bool Product::operator==(const Product& other)const
{
    return (this->text(1) == other.text(1));
}
