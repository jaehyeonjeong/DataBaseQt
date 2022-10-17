#ifndef CHETTING_H
#define CHETTING_H

#include <QWidget>
#include <QDataStream>

class QTextEdit;
class QLineEdit;
class QTcpSocket;
class QTcpServer;
class QTreeWidgetItem;
class QListWidgetItem;

typedef struct{
    int type;
    char data[1020];
}chattingProtocol;

namespace Ui {
class Chetting;
}

class Chetting : public QWidget
{
    Q_OBJECT

public:
    explicit Chetting(QWidget *parent = nullptr);

    ~Chetting();

private slots:
    /*server slots*/
    void clientConnect();
    void echoData();
    void removeItem();

    /*client slots and protocol*/
    void connectToServer( );
    void receiveData( );			// 서버에서 데이터가 올 때
    void sendData( );			// 서버로 데이터를 보낼 때
    void disconnect( );

    /*tcp client로 받는 데이터를 리시브 할 수 있는 코그*/
    void receiveTCPClientName(QString name);

    void on_reduceclient_clicked();

    void receiveClient(QString);

    void on_listWidget_itemClicked(QListWidgetItem *item);

private:
    void closeEvent(QCloseEvent*) override;
    Ui::Chetting *ui;
    /*tcp server*/
    QTcpServer* tcpServer;
    QList<QTcpSocket*> serverList;

    /*tcp client*/
    QTcpSocket* clientSocket;
    //QList<QTcpSocket*> clientList;
};

#endif // CHETTING_H
