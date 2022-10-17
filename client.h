#ifndef CLIENT_H
#define CLIENT_H

#include <QTreeWidgetItem>

class Client : public QTreeWidgetItem
{
public:
    Client(int id = 0, QString = "", QString = "", QString = "");

    QString getName() const;
    void setName(QString&);
    QString getPhoneNumber() const;
    void setPhoneNumber(QString&);
    QString getAddress() const;
    void setAddress(QString&);
    int id() const;
    bool operator==(const Client &other) const;
};

#endif // CLIENT_H
