#ifndef SHOPPING_H
#define SHOPPING_H

#include <QTreeWidgetItem>

class Shopping : public QTreeWidgetItem
{
public:
    Shopping(int id = 0, QString = "", QString = "", QString = "", int quan = 0, int AllPrice = 0);
                    //Shopping 생성자 초기화
    int getId();                    /*구매정보 등록 id*/
    QString getClientName();
    void setClientName(QString&);   /*고객의 성함을 받는 함수*/
    QString getProductName();
    void setProductName(QString&);  /*상품의 이름을 받는 함수*/
    QString getDate();
    void setDate(QString&);         /*날짜 정보 함수*/
    int getquan();
    void setquan(int&);             /*상품의 가격을 받는 함수*/
    int getAllPrice();
    void setAllPrice(int&);         /*상품의 총 가격(수량 X 해당 상품의 가격)*/
};

#endif // SHOPPING_H
