#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include <QList>
#include <QHash>

//#include <QLabel>
//#include <QPushButton>
//#include <QTcpServer>
//#include <QTcpSocket>
class QLabel;
class QTcpServer;
class QTcpSocket;

namespace Ui{
class tcplog;
}

typedef enum {          /*chat state*/
    Server_Chat_Login,     //로그인           -->초대를 위한 정보 저장
    Server_Chat_In,        //채팅방 입장
    Server_Chat_Talk,      //채팅
    Server_Chat_Close,     //채팅방 입장        --> 초대 가능
    Server_Chat_LogOut,    //로그 아웃 서버     --> 초대 불가능
    Server_Chat_Invite,    //초대
    Server_Chat_KickOut,   //강제퇴장
    Server_Chat_FileTrasform_Start,    //파일전송시작 --> 파일오픈
    Server_Chat_FileTransform,         //파일전송
    Server_Chat_FileTrasform_End,      //파일전송해제 --> 파일닫기
} Chat_Status;

typedef struct {        /*데이터를 쏠때 구조체로 데이터를 보냄*/
    Chat_Status type;   /*chat state의 값만 받을 수 있음*/
    char data[1020];
} chatProtocol;


class TCPServer : public QWidget
{
    Q_OBJECT
public:
    explicit TCPServer(QWidget *parent = nullptr);
    ~TCPServer();

signals:
private slots:
    void clientConnect();
    void receiveData();
//    void echoData();
//    void removeItem();
    void removeClient( );
    void addClient(int, QString);
    void inviteClient();
    void kickOut();
    //void on_clientTreeWidget_customContextMenuRequested(const QPoint &pos);

    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

private:
    const int BLOCK_SIZE = 1024;
    Ui::tcplog *ui;
    //QLabel* infoLabel;        //tcpserver의 정보를 불러오는 에디트
    QTcpServer* tcpServer;      /*같음*/
    QList<QTcpSocket*> clientList;  /*같음*/
    QList<int> clientIDList;
    QHash<QString, QString> clientNameHash;
    QHash<QString, int> clientIDHash;
    QMenu* menu;
};

#endif // TCPSERVER_H
