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

/*1019 파일과 파일 전송 상태를 나타내는 프로그래스 바*/
class QFile;
class QProgressDialog;
class LogThread;

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
} Chat_Status;


class TCPServer : public QWidget
{
    Q_OBJECT
public:
    explicit TCPServer(QWidget *parent = nullptr);
    ~TCPServer();

signals:
    void SendLogInChecked(int);     /*비교 슬롯의 데이터를 받으면 다시 클라이언트로 보내는 signal*/

private slots:
    void acceptConnection();            /*파일 서버*/
    void readClient();

    //////////////////////////////////////////////////////////////////////////////////

    void clientConnect();                /*채팅 서버*/
    void receiveData();
//    void echoData();
//    void removeItem();
    void removeClient( );

    /*클라이언트 데이터가 추가, 삭제, 수정 될때마다 서버의 클라이언트 리스트가 바뀜*/
    void addClient(int, QString);
    void removeClient(int);
    void modifyClient(int, QString, int);

    /*chetting class에 있는 데이터가 받아지는 함수*/
    void receiveManager(int, QString);


    void inviteClient();
    void kickOut();
    //void on_clientTreeWidget_customContextMenuRequested(const QPoint &pos);

    /*클라이언트 데이터를 초대 및 강퇴 하기 위한 이벤트 핸들러*/
    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

    /*시그널을 받는 슬롯을 제작*/
    void CheckLogIn(QString);

private:
    const int BLOCK_SIZE = 1024;    /*1019 채팅 일력 제한수*/
    const int PORT_NUMBER = 8000;
    Ui::tcplog *ui;
    //QLabel* infoLabel;        //tcpserver의 정보를 불러오는 에디트
    QTcpServer* tcpServer;      /*채팅만 받는 서버*/
    QTcpServer* fileServer;     /*파일만 받는 서버 추가*/

    QList<QTcpSocket*> clientList;  /*같음*/
    QList<int> clientIDList;        /*클라이언트 트리 위젯에 나열할 STL*/
    QHash<quint64, QString> clientNameHash; /*틀라이언트 이름을 받는 해시 데이터*/
    QHash<QString, QTcpSocket*> clientSocketHash; /*클라이언트 소켓 해시 데이터*/
    QHash<QString, int> clientIDHash;   /*클라이언트 ID를 받는 해시 데이터*/
    QMenu* menu;        /*마우스 우클릭시 생성되는 메뉴창*/

    /*1019 강사님께서 추가해주신 코드*/
    QFile* file;                /*파일 생성 변수*/
    QProgressDialog* progressDialog;    /*프로그래스바 */
    qint64 totalSize;           /*데이터 토탈 사이즈 변수*/
    qint64 byteReceived;       /*데이터 받기 변수*/
    /*1020 코드수정*/
    QByteArray inBlock;
    LogThread* logThread;

    /*받아오는 이름과 아이디*/
    QString CName;
    int CId;
};

#endif // TCPSERVER_H
