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

/*1019 코드 수정*/
class QFile;
class QProgressDialog;
class QByteArray;

typedef enum {          /*chat state*/
    Chat_Login,     //로그인           -->초대를 위한 정보 저장
    Chat_In,        //채팅방 입장
    Chat_Talk,      //채팅
    Chat_Close,     //채팅방 퇴장       --> 초대 가능
    Chat_LogOut,    //로그 아웃 서버     --> 초대 불가능
    Chat_Invite,    //초대
    Chat_KickOut,   //강제퇴장

    /*같은 서버에서 통신하지 않게 지워두기 1020코드수정*/
//    Chat_FileTrasform_Start,    //파일전송시작 --> 파일오픈
//    Chat_FileTransform,         //파일전송
//    Chat_FileTrasform_End,      //파일전송해제 --> 파일닫기
} StatusOfChat;

/*1019 강사님 코드에는 구조체가없음*/
//typedef struct{
//    StatusOfChat type;
//    char data[1020];
//}chattingProtocol;

namespace Ui {
class Chetting;
}

class Chetting : public QWidget
{
    Q_OBJECT

public:
    const int PORT_NUMBER = 8000;

    explicit Chetting(QWidget *parent = nullptr);
    ~Chetting();

signals:
    void TCPSignal(int, QString);

private slots:
    /*server slots*/
//    void clientConnect();
//    void echoData();
//    void removeItem();

    /*client slots and protocol*/
    //void connectToServer( );
    void receiveData( );			// 서버에서 데이터가 올 때
    void sendData( );			// 서버로 데이터를 보낼 때
    void disconnect( );

    /*파일및 프로토콜을 보내는 슬롯*/
    void sendFile();
    void sendProtocol(StatusOfChat, char*, int = 1020);
    void goOnSend(qint64);

    /*tcp client로 받는 데이터를 리시브 할 수 있는 코드*/
    void receiveTCPClientName(QString);



    //void on_listWidget_itemClicked(QListWidgetItem *item);

private:
    void closeEvent(QCloseEvent*) override;
    Ui::Chetting *ui;
    /*tcp server*/
//    QTcpServer* tcpServer;
//    QList<QTcpSocket*> serverList;

    /*tcp client*/
    QTcpSocket* clientSocket;           /*클라이언트용 소켓*/
    QTcpSocket* fileSocket;             /*파일 전용 소켓*/

    /*1019 변수 추가*/ /*1020 코드수정*/
    QProgressDialog* progressDialog;    // 파일 진행 확인
    QFile* file;
    qint64 loadSize;
    qint64 byteToWrite;
    qint64 totalSize;
    QByteArray outBlock;

    bool isSent;


    //QList<QTcpSocket*> clientList;
};

#endif // CHETTING_H
