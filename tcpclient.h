#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDataStream>


class QTextEdit;
class QLineEdit;
class QTcpSocket;
class QPushButton;
class QFile;
class QLabel;
class QPixmap;
class QProgressDialog;
class clientThread;
class QSqlTableModel;
class QSqlQuery;
class QTreeWidget;

typedef enum {
    Client_Chat_Login,             // 로그인(서버 접속)   --> 초대를 위한 정보 저장
    Client_Chat_In,                // 채팅방 입장
    Client_Chat_Talk,              // 채팅
    Client_Chat_Out,             // 채팅방 퇴장         --> 초대 가능
    Client_Chat_LogOut,            // 로그 아웃(서버 단절) --> 초대 불가능
    Client_Chat_Invite,            // 초대
    Client_Chat_KickOut,           // 강퇴
} Client_Chat;

class TCPClient : public QWidget
{
    Q_OBJECT

public:
    const int PORT_NUMBER = 8000;   //연결할 포트 번호 설정

    TCPClient(QWidget *parent = nullptr); //클라이언트 생성자 창
    ~TCPClient();

    /*채팅방에서 없는 데이터가 채팅이 되는 오류를 방지하기 위한 시그널*/
signals:
    void compareName(QString);


private slots:
    void receiveData( );			// 서버에서 데이터가 올 때
    void sendData( );               // 서버로 데이터를 보낼 때
    void disconnect( );             // 연결이 차단되는 슬롯
    void sendProtocol(Client_Chat, char*, int = 1020); //프로토콜 타입 슬롯 (4바이트) 나머지 1020
    void sendFile();                //보내는 파일 슬롯
    void goOnSend(qint64);          //파일 데이터 전송 함수

    /*관리자가 보내준 파일을 받는 슬롯*/
    void filereceive();

    /*비교하는 시그널을 받는 슬롯*/
    void receiveSignal(int);

private:
    void closeEvent(QCloseEvent*) override; /*이벤트 핸들러 적용*/

    int flag = 0;

    QLineEdit *name;                // ID(이름)을 입력하는 창
    QTextEdit *message;             // 서버에서 오는 메세지 표시용
    QLineEdit* serverAddress;       // Ip주소 메세지 표지줄
    QLineEdit* serverPort;          // 서버 포트 표시줄
    QLineEdit *inputLine;           // 서버로 보내는 메시지 입력용
    QPushButton *connectButton;     // 서버 로그인 등 접속 처리
    QPushButton *sentButton;        // 메시지 전송
    //QPushButton* fileButton;        // 파일 전송
    QTcpSocket *clientSocket;		// 클라이언트용 소켓

    QPushButton *findFileButton;    //파일을 찾는 버튼
    QTextEdit *fileText;            //읽어오는 파일 텍스트


    /*파일 및 가변수*/
    QTcpSocket *fileClient;         //파일을 보내는 소켓 클래스 변수
    QProgressDialog* progressDialog;    // 파일 진행 확인
    QFile* file;                    // 파일 변수
    qint64 loadSize;
    qint64 byteToWrite;
    qint64 totalSize;
    QByteArray outBlock;            /*cpp에서 자세히 설명*/
    bool isSent;
    int result;                 /*없는 고객이름이 채팅이 되지 않도록 잡아주는 변수*/

    /*이미지 파일 불러오기 변주*/

    QPushButton* imageButton;
    QLabel* IbView;

    /*QTreeWidget widget varient*/
    QTreeWidget* treeWidget;

    /*clientThread Varient*/
    clientThread* clientTh;

    QSqlTableModel* ClientModel;
    QSqlQuery* clientquery;
};
#endif // WIDGET_H
