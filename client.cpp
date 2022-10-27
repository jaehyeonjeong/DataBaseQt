#include "client.h"

    /*QTreeWidgetItem을 상속받았기 때문에 setText(column, text)형태로 데이터를 저장*/
Client::Client(int id, QString name, QString phoneNumber, QString address)
{
    setText(0, QString::number(id));        /*1번째 에디터에 들어가는 정보*/
    setText(1, name);                       /*2번째 ""*/
    setText(2, phoneNumber);                /*3번째 ""*/
    setText(3, address);                    /*4번째 ""*/
}

int Client::id() const
{
    return text(0).toInt(); /*treeWidget column 1*/
}

QString Client::getName() const
{
    return text(1);     /*treeWidget column 2*/
}

void Client::setName(QString& name)
{
    setText(1, name);
}

QString Client::getPhoneNumber() const
{
    return text(2);     /*treeWidget column 3*/
}

void Client::setPhoneNumber(QString& phoneNumber)
{
    setText(2, phoneNumber);    // c_str() --> const char*
}

QString Client::getAddress() const
{
    return text(3);     /*treeWidget column 4*/
}

void Client::setAddress(QString& address)
{
    setText(3, address);
}

// Define copy assignment operator.
bool Client::operator==(const Client &other) const {
    return (this->text(1) == other.text(1));
}
