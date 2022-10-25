#include "tcpclient.h".h"

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
#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QtNetwork>
#include <QProgressDialog>

#define BLOCK_SIZE      1024

TCPClient::TCPClient(QWidget *parent) : QWidget(parent), isSent(false) {
    // 연결한 서버 정보 입력을 위한 위젯들
    name = new QLineEdit(this);

    /*ipAddress 자동 할당 코드*/
    QString ipAddress;
    QNetworkInterface interface;
    QList<QHostAddress> ipList=interface.allAddresses();
    for (int i = 0; i < ipList.size(); i++)
    {
        if (ipList.at(i) != QHostAddress::LocalHost && ipList.at(i).toIPv4Address())
        {
            ipAddress = ipList.at(i).toString();
            break;
        }
    }
    /*ipAddress 자동 할당 코드*/

    serverAddress = new QLineEdit(this);
    serverAddress->setText(ipAddress);
    //serverAddress->setInputMask("999.999.999.999;_");
    QRegularExpression re("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegularExpressionValidator validator(re);
    serverAddress->setPlaceholderText("Server IP Address");
    serverAddress->setValidator(&validator);

    serverPort = new QLineEdit(this);
    serverPort->setText(QString::number(PORT_NUMBER));
    serverPort->setInputMask("00000;_");
    serverPort->setPlaceholderText("Server Port No");

    connectButton = new QPushButton(tr("Log In"), this);
    QHBoxLayout *serverLayout = new QHBoxLayout;
    serverLayout->addWidget(name);
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
    sentButton = new QPushButton("Send", this);
    connect(sentButton, SIGNAL(clicked( )), SLOT(sendData( )));
    connect(sentButton, SIGNAL(clicked( )), inputLine, SLOT(clear( )));
    inputLine->setEnabled(false);
    sentButton->setEnabled(false);

    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(inputLine);
    inputLayout->addWidget(sentButton);

    fileButton = new QPushButton("File Transfer", this);
    connect(fileButton, SIGNAL(clicked( )), SLOT(sendFile( )));
    fileButton->setDisabled(true);

    // 종료 기능
    QPushButton* quitButton = new QPushButton("Log Out", this);
    connect(quitButton, SIGNAL(clicked( )), this, SLOT(close( )));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(fileButton);
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

    QSettings settings("ChatClient", "Chat Client");
    name->setText(settings.value("ChatClient/ID").toString());

    fileClient = new QTcpSocket(this);
    connect(fileClient, SIGNAL(bytesWritten(qint64)), SLOT(goOnSend(qint64)));

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();

    connect(connectButton, &QPushButton::clicked,
            [=]{
        if(connectButton->text() == tr("Log In")) {
            emit compareName(name->text().toStdString().data());             /*커넥트 버튼 누를 시 발송되는 시그널*/
            qDebug() << name->text().toStdString().data();
            clientSocket->connectToHost(serverAddress->text( ),
                                        serverPort->text( ).toInt( ));
            clientSocket->waitForConnected();
            sendProtocol(Client_Chat_Login, name->text().toStdString().data());
            connectButton->setText(tr("Chat in"));
            name->setReadOnly(true);
        } else if(connectButton->text() == tr("Chat in"))  {
            sendProtocol(Client_Chat_In, name->text().toStdString().data());
            connectButton->setText(tr("Chat Out"));
            inputLine->setEnabled(true);
            sentButton->setEnabled(true);
            fileButton->setEnabled(true);
        } else if(connectButton->text() == tr("Chat Out"))  {
            sendProtocol(Client_Chat_Out, name->text().toStdString().data());
            connectButton->setText(tr("Chat in"));
            inputLine->setDisabled(true);
            sentButton->setDisabled(true);
            fileButton->setDisabled(true);
        }
    } );

    setWindowTitle(tr("Chat Client"));
}

void TCPClient::receiveSignal(int num)
{
    result = num;
}

TCPClient::~TCPClient( )
{
    clientSocket->close( );
    QSettings settings("ChatClient", "Chat Client");
    settings.setValue("ChatClient/ID", name->text());
}

void TCPClient::closeEvent(QCloseEvent*)
{
    sendProtocol(Client_Chat_LogOut, name->text().toStdString().data());
    clientSocket->disconnectFromHost();
    if(clientSocket->state() != QAbstractSocket::UnconnectedState)
        clientSocket->waitForDisconnected();
}

void TCPClient::receiveData( )
{
    QTcpSocket *clientSocket = dynamic_cast<QTcpSocket *>(sender( ));
    if (clientSocket->bytesAvailable( ) > BLOCK_SIZE) return;
    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);

    Client_Chat type;       // 채팅의 목적

    QString ClientName;

    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in.device()->seek(0);

    char data[1020 /*- sizeof(ClientName)*/];        // 전송되는 메시지/데이터
    memset(data, 0, 1020 /*- sizeof(ClientName)*/);
    in >> type /*>> ClientName*/;
    in.readRawData(data, 1020 /*- sizeof(ClientName)*/);

    switch(type) {
    case Client_Chat_Talk:

        //qDebug("[%s] %s : %d", __FILE__, __FUNCTION__, __LINE__);
        if(TCPClient::flag == 0)    /*플래그를 설정해서 0인경우 채팅 활성화*/
        {
            message->append(QString(data));
            //inputLine->setDisabled(true);
            inputLine->setEnabled(true);
            sentButton->setEnabled(true);
            fileButton->setEnabled(true);
            //connectButton->setText("Chat Out");
            connectButton->setEnabled(false);
        }
        else    /*한번 강퇴 되면 플래그가 1로 변경되어서 입력문에 채팅을 할 수 없게 됨*/
        {
            inputLine->setDisabled(true);
            inputLine->setEnabled(false);
            sentButton->setEnabled(false);
            fileButton->setEnabled(false);
            //connectButton->setText("Chat Out");
            connectButton->setEnabled(false);
        }
        break;
    case Client_Chat_KickOut:
        flag = 1;
        QMessageBox::critical(this, tr("Chatting Client"), \
                              tr("Kick out from Server"));
        inputLine->setDisabled(true);
        sentButton->setDisabled(true);
        fileButton->setDisabled(true);
        connectButton->setText("Chat in");
        connectButton->setEnabled(false);
        name->setReadOnly(false);
        break;
    case Client_Chat_Invite:
        flag = 0;
        QMessageBox::information(this, tr("Chatting Client"), \
                              tr("Invited from Server"));
        /*프로토콜을 보내서 챗인 상태로 전환하는 좋은 방법*/
//        sendProtocol(Client_Chat_In,
//                     ClientName.toStdString().data());
        inputLine->setEnabled(true);
        sentButton->setEnabled(true);
        fileButton->setEnabled(true);
        connectButton->setText("Chat Out");
        connectButton->setEnabled(true);
        name->setReadOnly(true);

        break;
    };
}

void TCPClient::disconnect( )
{
    QMessageBox::critical(this, tr("Chatting Client"), \
                          tr("Disconnect from Server"));
    inputLine->setEnabled(false);
    name->setReadOnly(false);
    sentButton->setEnabled(false);
    connectButton->setText(tr("Log in"));
}

void TCPClient::sendProtocol(Client_Chat type, char* data, int size)
{
    if(result == 1)
    {
        QByteArray dataArray;           // 소켓으로 보낼 데이터를 채우고
        QDataStream out(&dataArray, QIODevice::WriteOnly);
        /*현재 설정된 I/O 장치를 반환하거나 현재 설정된 장치가 없으면 nullptr을 반환합니다.*/
        out.device()->seek(0);          //버퍼를 맨앞에다가 위치
        /*QIODevice를 서브클래싱할 때 QIODevice의 내장 버퍼와의
         * 무결성을 보장하기 위해 함수 시작 시 QIODevice::seek()를 호출해야 합니다.*/
        out << type;
        out.writeRawData(data, size);
        clientSocket->write(dataArray);     // 서버로 전송
        clientSocket->flush();
        while(clientSocket->waitForBytesWritten());
    }
    else
    {
        return;
    }
}

void TCPClient::sendData(  )
{
    QString str = inputLine->text( );
    if(str.length( )) {
        QByteArray bytearray;
        bytearray = str.toUtf8( );
        message->append("<font color=red>나</font> : " + str);
        sendProtocol(Client_Chat_Talk, bytearray.data());
    }
}

void TCPClient::goOnSend(qint64 numBytes) // 파일 내용 보내기 시작
{
    byteToWrite -= numBytes; // Remaining data size
    outBlock = file->read(qMin(byteToWrite, numBytes));
    fileClient->write(outBlock);

    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(totalSize-byteToWrite);

    if (byteToWrite == 0) { // Send completed
        qDebug("File sending completed!");
        progressDialog->reset();
    }
}

void TCPClient::sendFile() // 파일을 열고 파일 이름(경로 포함)을 가져옵니다.
{
    loadSize = 0;
    byteToWrite = 0;
    totalSize = 0;
    outBlock.clear();

    QString filename = QFileDialog::getOpenFileName(this);
    file = new QFile(filename);
    file->open(QFile::ReadOnly);

    qDebug() << QString("file %1 is opened").arg(filename);
    progressDialog->setValue(0); // 처음으로 전송되지 않음

    if (!isSent) { //처음 전송될 때만 연결이 연결 신호를 생성할 때 발생합니다.

        fileClient->connectToHost(serverAddress->text( ),
                                  serverPort->text( ).toInt( ) + 1);
        isSent = true;
    }

    // 처음으로 보낼 때 connectToHost는 send를 호출하기 위해 연결 신호를 시작하고 두 번째 후에는 send를 호출해야 합니다.

    byteToWrite = totalSize = file->size(); // 나머지 데이터의 크기
    loadSize = 1024; // 매번 전송되는 데이터의 크기

    QDataStream out(&outBlock, QIODevice::WriteOnly);
    out << qint64(0) << qint64(0) << filename;

    totalSize += outBlock.size(); // 전체 크기는 파일 크기에 파일 이름 및 기타 정보의 크기를 더한 것입니다.

    byteToWrite += outBlock.size();
    /*데이터 스트림의 커서를 앞에 위치 시킴*/
    out.device()->seek(0); //바이트 스트림의 시작 부분으로 돌아가 전체 크기와 파일 이름 및 기타 정보 크기인 qint64를 앞에 씁니다.
    out << totalSize << qint64(outBlock.size());

    fileClient->write(outBlock); // 읽은 파일을 소켓으로 보냅니다.

    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(totalSize-byteToWrite);
    progressDialog->show();

    qDebug() << QString("Sending file %1").arg(filename);
}
