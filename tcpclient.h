#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QDataStream>

class QTextEdit;
class QLineEdit;
class QTcpSocket;
class QPushButton;


typedef enum {          /*chat state*/
    TCPChat_Login,     //로그인           -->초대를 위한 정보 저장
    TCPChat_In,        //채팅방 입장
    TCPChat_Talk,      //채팅
    TCPChat_Close,     //채팅방 입장        --> 초대 가능
    TCPChat_LogOut,    //로그 아웃 서버     --> 초대 불가능
    TCPChat_Invite,    //초대
    TCPChat_KickOut,   //강제퇴장
    TCPChat_FileTrasform_Start,    //파일전송시작 --> 파일오픈
    TCPChat_FileTransform,         //파일전송
    TCPChat_FileTrasform_End,      //파일전송해제 --> 파일닫기
} TCPChat_Status;


typedef struct {        /*데이터를 쏠때 구조체로 데이터를 보냄*/
    TCPChat_Status type;   /*chat state의 값만 받을 수 있음*/
    char data[1020];
} TCPProtocolType;

class TCPClient : public QWidget
{
    Q_OBJECT

public:
    TCPClient(QWidget *parent = nullptr);
    ~TCPClient();

signals:
    void ButtonSignal(QString);

private slots:
    void connectToServer( );
    void receiveData( );			// 서버에서 데이터가 올 때
    void sendData( );			// 서버로 데이터를 보낼 때
    void disconnect( );

    void CReceiveData(QString);
private:
    const int BLOCK_SIZE = 1024;
    void closeEvent(QCloseEvent*) override;     /*이벤트 헨들러 사용*/

    QLineEdit *name;        // 클라이언트 이름을 입력받는 에디트
    QLineEdit *IDNumber;    // 클라이언트 ID를 입력받는 에디트
    QTextEdit *message;		// 서버에서 오는 메세지 표시용
    QLineEdit *inputLine;		// 서버로 보내는 메시지 입력용
    QPushButton* sentButton;
    QPushButton* connectButton;
    QTcpSocket *clientSocket;		// 클라이언트용 소켓
};

#endif // TCPCLIENT_H
