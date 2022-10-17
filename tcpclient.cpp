#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDataStream>
#include <QTcpSocket>
#include <QApplication>
#include <QThread>
#include <QMessageBox>

#include "tcpclient.h"

#define BLOCK_SIZE 1024

TCPClient::TCPClient(QWidget *parent)
    : QWidget(parent)
{
    serverName = new QLineEdit(this);

    QLineEdit* serverAddress = new QLineEdit(this);
    serverAddress->setText("127.0.0.1");
    //    serverAddress->setInputMask("999.999.999.999;_");
    QRegularExpression re("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegularExpressionValidator validator(re);
    serverAddress->setPlaceholderText("Server IP Address");
    serverAddress->setValidator(&validator);

    QLineEdit* serverPort = new QLineEdit(this);
    serverPort->setText("8000");
    serverPort->setInputMask("00000;_");
    serverPort->setPlaceholderText("Server Port No");

    QPushButton* connectButton = new QPushButton("connect", this);
    connect(connectButton, &QPushButton::clicked,
            [=]{ clientSocket->connectToHost(serverAddress->text( ),
                                             serverPort->text( ).toInt( ));
        connectToServer( ); } );

    QHBoxLayout *serverLayout = new QHBoxLayout;
    serverLayout->addWidget(serverName);
    serverLayout->addStretch(1);
    serverLayout->addWidget(serverAddress);
    serverLayout->addWidget(serverPort);
    serverLayout->addWidget(connectButton);

    message = new QTextEdit(this);		// 서버에서 오는 메시지 표시용
    message->setReadOnly(true);

    // 서버로 보낼 메시지를 위한 위젯들
    inputLine = new QLineEdit(this);
    connect(inputLine, SIGNAL(returnPressed( )), SLOT(sendData( )));
    connect(inputLine, SIGNAL(returnPressed( )), inputLine, SLOT(clear( )));
    QPushButton* sentButton = new QPushButton("Send", this);
    connect(sentButton, SIGNAL(clicked( )), SLOT(sendData( )));
    connect(sentButton, SIGNAL(clicked( )), inputLine, SLOT(clear( )));

    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(inputLine);
    inputLayout->addWidget(sentButton);

    // 종료 기능
    QPushButton* quitButton = new QPushButton("Quit", this);
    connect(quitButton, SIGNAL(clicked( )), qApp, SLOT(quit( )));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(serverLayout);
    mainLayout->addWidget(message);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    clientSocket = new QTcpSocket(this);			// 클라이언트 소켓 생성
    connect(clientSocket, &QAbstractSocket::errorOccurred,
            [=]{ qDebug( ) << clientSocket->errorString( ); });
    connect(clientSocket, SIGNAL(readyRead( )), SLOT(receiveData( )));
    connect(clientSocket, SIGNAL(disconnected( )), SLOT(disconnect( )));
    setWindowTitle(tr("Chat Client"));

    /*client data 보내기*/
    connect(connectButton, SIGNAL(clicked()), this, SIGNAL(ButtonSignal(QString)));
    connect(serverName, SIGNAL(textChanged(QString)), this,
            SIGNAL(clientName(QString)));
//    connect(serverName, SIGNAL(textChanged(QString)),
//            this, SIGNAL(ClientSignal(QString));
}

TCPClient::~TCPClient()
{
     clientSocket->close( );
}

void TCPClient::CReceiveData(QString str)
{
    serverName->setText(str);
}

void TCPClient::closeEvent(QCloseEvent*)
{
    chatProtocolType data;
    data.type = 3;
    qstrcpy(data.data, serverName->text().toStdString().data());

    QByteArray sendArray;
    QDataStream out(&sendArray, QIODevice::WriteOnly);
    out << data.type;
    out.writeRawData(data.data, 1020);
    clientSocket->write(sendArray);

    clientSocket->disconnectFromHost();
    while(clientSocket->waitForDisconnected(1000))
        QThread::usleep(10);
}

void TCPClient::receiveData( )
{
    QTcpSocket *clientSocket = dynamic_cast<QTcpSocket *>(sender( ));
    if (clientSocket->bytesAvailable( ) > BLOCK_SIZE) return;
    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);
    chatProtocolType data;
    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in >> data.type;
    in.readRawData(data.data, 1020);

    qDebug( ) << data.type;
    switch(data.type) {
    case 2:
        message->append(QString(data.data));
        break;
    case 4:
        QMessageBox::critical(this, tr("Chatting Client"), \
                              tr("Invited from Server"));
        break;
    };
}

void TCPClient::sendData(  )
{
    QString str = inputLine->text( );
    if(str.length( )) {
        QByteArray bytearray;
        bytearray = str.toUtf8( );
        message->append(QString("<font color=red>%1</font> : ")
                        .arg(serverName->text())+ str);

        chatProtocolType data;
        data.type = 2;
        qstrcpy(data.data, bytearray.data());

        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << data.type;
        out.writeRawData(data.data, 1020);
        clientSocket->write(sendArray);


    }
}

void TCPClient::connectToServer( )
{
    chatProtocolType data;
    data.type = 1;
    qstrcpy(data.data, serverName->text().toStdString().data());

    QByteArray sendArray;
    QDataStream out(&sendArray, QIODevice::WriteOnly);
    out << data.type;
    out.writeRawData(data.data, 1020);
    clientSocket->write(sendArray);

    /*send button을 누를시 보내지는 시그널*/
    emit ButtonSignal(serverName->text().toStdString().data());
}

void TCPClient::disconnect( )
{
    QMessageBox::critical(this, tr("Chatting Client"), \
                          tr("Disconnect from Server"));
//    close( );
}
