#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>

#include <QLabel>
#include <QPushButton>
#include <QTcpServer>
#include <QTcpSocket>

class TCPServer : public QWidget
{
    Q_OBJECT
public:
    explicit TCPServer(QWidget *parent = nullptr);

signals:
private slots:
    void clientConnect();
    void echoData();
    void removeItem();

private:
    QLabel* infoLabel;
    QTcpServer* tcpServer;
    QList<QTcpSocket*> clientList;
};

#endif // TCPSERVER_H
