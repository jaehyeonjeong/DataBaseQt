#ifndef PRODUCT_H
#define PRODUCT_H

#include <QTreeWidgetItem>

class Product : public QTreeWidgetItem
{
public:
    Product(int id = 0, QString = "", QString = "", int price = 0); /*Product 생성자 초기화*/
    int getid();                    /*상품의 id함수 생성*/
    QString getName();
    void setName(QString &);        /*상품의 이름정보 입/출력 함수*/
    QString getCompany();
    void setCompany(QString &);     /*상품의 회사정보 입/출력 함수*/
    int getPrice();
    void setPrice(int &);           /*상품의 가격정보 입/출력 함수*/
    bool operator==(const Product& other)const;
};

#endif // PRODUCT_H
