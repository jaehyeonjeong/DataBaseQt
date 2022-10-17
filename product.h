#ifndef PRODUCT_H
#define PRODUCT_H

#include <QTreeWidgetItem>

class Product : public QTreeWidgetItem
{
public:
    Product(int id = 0, QString = "", QString = "", int price = 0);
    int getid();
    QString getName();
    void setName(QString &);
    QString getCompany();
    void setCompany(QString &);
    int getPrice();
    void setPrice(int &);
    bool operator==(const Product& other)const;
};

#endif // PRODUCT_H
