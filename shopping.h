#ifndef SHOPPING_H
#define SHOPPING_H

#include <QTreeWidgetItem>

class Product;
class Client;
class ProductManager;

class Shopping : public QTreeWidgetItem
{
public:
    Shopping(int id = 0, QString = "", QString = "", QString = "", int quan = 0, int AllPrice = 0);
    int getId();
    QString getClientName();
    void setClientName(QString&);
    QString getProductName();
    void setProductName(QString&);
    QString getDate();
    void setDate(QString&);
    int getquan();
    void setquan(int&);
    int getAllPrice();
    void setAllPrice(int&);
};

#endif // SHOPPING_H
