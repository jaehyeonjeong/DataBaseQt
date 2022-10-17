#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QDataStream>


class QTextEdit;
class QLineEdit;
class QTcpSocket;

typedef struct{
    int type;
    char data[1020];
}chatProtocolType;

class TCPClient : public QWidget
{
    Q_OBJECT
public:
    explicit TCPClient(QWidget *parent = nullptr);
    ~TCPClient();

signals:
    void ButtonSignal(QString);
    void clientName(QString);

private slots:
    void connectToServer( );
    void receiveData( );			// 서버에서 데이터가 올 때
    void sendData( );			// 서버로 데이터를 보낼 때
    void disconnect( );

    /*client Name Receive slot*/
    void CReceiveData(QString str);


private:
    void closeEvent(QCloseEvent*) override;

    QLineEdit *serverName;
    QTextEdit *message;		// 서버에서 오는 메세지 표시용
    QLineEdit *inputLine;		// 서버로 보내는 메시지 입력용
    QTcpSocket *clientSocket;		// 클라이언트용 소켓
};

#endif // TCPCLIENT_H
