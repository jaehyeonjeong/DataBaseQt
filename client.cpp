#include "client.h"

Client::Client(int id, QString name, QString phoneNumber, QString address)
{
    setText(0, QString::number(id));
    setText(1, name);
    setText(2, phoneNumber);
    setText(3, address);
}

QString Client::getName() const
{
    return text(1);
}

void Client::setName(QString& name)
{
    setText(1, name);
}

QString Client::getPhoneNumber() const
{
    return text(2);
}

void Client::setPhoneNumber(QString& phoneNumber)
{
    setText(2, phoneNumber);    // c_str() --> const char*
}

QString Client::getAddress() const
{
    return text(3);
}

void Client::setAddress(QString& address)
{
    setText(3, address);
}

int Client::id() const
{
    return text(0).toInt();
}

// Define copy assignment operator.
bool Client::operator==(const Client &other) const {
    return (this->text(1) == other.text(1));
}
