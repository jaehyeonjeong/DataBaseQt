#ifndef CLIENT_H
#define CLIENT_H

#include <QTreeWidgetItem>

class Client : public QTreeWidgetItem
{
public:
    Client(int id = 0, QString = "", QString = "", QString = "");   /*Client 생성자 초기화*/

    QString getName() const;
    void setName(QString&);             /*고객 성함 정보 입/출력 함수*/
    QString getPhoneNumber() const;
    void setPhoneNumber(QString&);      /*고객 전화번호 정보 입/출력 함수*/
    QString getAddress() const;
    void setAddress(QString&);          /*고객 이메일주소 정보 입/출력 함수*/
    int id() const;                     /*고객의 ID함수 생성*/
    bool operator==(const Client &other) const;
};

#endif // CLIENT_H
