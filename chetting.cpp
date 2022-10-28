#include "chetting.h"
#include "ui_chetting.h"
#include <QtNetwork>
#include <QMessageBox>
#include <QDataStream>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
//#include <QNetworkAccess>
#include <QDateTime>
#include <QThread>

/*1019 추가 헤더 파일  1020 코드 수정*/
#include <QProgressDialog>
#include <QFile>
#include <QFileDialog>
#include <QTcpServer>

#define BLOCK_SIZE 1024

Chetting::Chetting(QWidget *parent) :
    QWidget(parent), ui(new Ui::Chetting),
    isSent(false)
{
    ui->setupUi(this);

    /*ipAddress 자동 할당 코드*/
    QString ipAddress;
    QNetworkInterface interface;
    QList<QHostAddress> ipList = interface.allAddresses();
    for (int i = 0; i < ipList.size(); i++)
    {
        if (ipList.at(i) != QHostAddress::LocalHost && ipList.at(i).toIPv4Address())
        {
            ipAddress = ipList.at(i).toString();
            break;
        }
    }
    /*ipAddress 자동 할당 코드*/

    /*tcp client*/
    ui->textmessage->setReadOnly(true);
    ui->ipAddress->setText(ipAddress);
    QRegularExpression re("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$|");

    QRegularExpressionValidator validator(re);
    ui->ipAddress->setPlaceholderText("Server IP Address");
    ui->ipAddress->setValidator(&validator);


    ui->portNumber->setText(QString::number(PORT_NUMBER));      /*8000으로 설정*/
    ui->portNumber->setInputMask("00000;_");
    ui->portNumber->setPlaceholderText("Server Port No");

    /*소켓도 항시 초기화가 필요함*/
    clientSocket = new QTcpSocket(this);  /*고객 채팅 전용 소켓*/


    /*초기 커넥트 버튼 화면*/
    ui->connectButton->setText("Log In");
    /*연결할 호스트 서버 연결*/
    connect(ui->connectButton,
            &QPushButton::clicked,
            [=]{
        if(ui->connectButton->text() == tr("Log In"))
        {
            /*관리자 이름 정보 전달*/
            emit TCPSignal(0, ui->manager->text());
            clientSocket->connectToHost(
                        ui->ipAddress->text(),
                        ui->portNumber->text().toInt());
            clientSocket->waitForConnected();      //서버 커넥트
            sendProtocol(Chat_Login, ui->manager->text().toStdString().data());
            ui->connectButton->setText("Chat in");
            ui->manager->setReadOnly(true);
            ui->serverstatus->setText("manager chetting Login");
        }
        else if(ui->connectButton->text() == tr("Chat in"))
        {
            sendProtocol(Chat_In, ui->manager->text().toStdString().data());
            ui->connectButton->setText(tr("Chat Out"));
            ui->inputEdit->setEnabled(true);
            ui->sendButton->setEnabled(true);
            ui->FileSendButton->setEnabled(true);
            ui->serverstatus->setText("manager chetting in");
        }
        else if(ui->connectButton->text() == tr("Chat Out"))
        {
            sendProtocol(Chat_Close, ui->manager->text().toStdString().data());
            ui->connectButton->setText(tr("Chat in"));
            ui->inputEdit->setDisabled(true);
            ui->sendButton->setDisabled(true);
            ui->FileSendButton->setDisabled(true);
            ui->serverstatus->setText("manager chetting out");
        }
    });

    fileSocket = new QTcpSocket(this);
    connect(fileSocket, SIGNAL(bytesWritten(qint64)), SLOT(goOnSend(qint64)));

    /*서버로 보낼 메시지를 위한 위젯들*/
    connect(ui->inputEdit, SIGNAL(returnPressed( )), SLOT(sendData( )));
    connect(ui->inputEdit, SIGNAL(returnPressed( )), ui->inputEdit, SLOT(clear( )));

    connect(ui->sendButton, SIGNAL(clicked( )), SLOT(sendData( )));
    connect(ui->sendButton, SIGNAL(clicked( )), ui->inputEdit, SLOT(clear( )));
    ui->inputEdit->setEnabled(false);
    ui->sendButton->setEnabled(false);

    /*파일 전송 버튼*/
    connect(ui->FileSendButton, SIGNAL(clicked()), SLOT(sendFile()));
    ui->FileSendButton->setDisabled(true);

    /*프로그래스 다이얼로그로 파일 진행상태 확인*/
    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();

    //서버로 보낼 메시지를 위한 위젯들
    connect(ui->inputEdit, SIGNAL(returnPressed()), SLOT(sendData()));
    connect(ui->inputEdit, SIGNAL(returnPressed()),
            ui->inputEdit, SLOT(clear()));

    connect(ui->sendButton, SIGNAL(clicked()), SLOT(sendData()));
    connect(ui->sendButton, SIGNAL(clicked()), ui->inputEdit, SLOT(clear()));

    connect(ui->cacelButton,
            &QPushButton::clicked,
            [=]{ui->inputEdit->setText("");});

    connect(clientSocket,
            &QAbstractSocket::errorOccurred,
            [=]{qDebug() << clientSocket->errorString();});
    connect(clientSocket, SIGNAL(readyRead()), SLOT(receiveData()));
    connect(clientSocket, SIGNAL(disconnected()), SLOT(disconnect()));

    //ui->IPNameEdit->setText("No names");

    ui->manager->setReadOnly(true);

    //해당 윈도우의 닫기 버튼을 비활성화 하는 함수
    //setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
}

Chetting::~Chetting()
{
    delete ui;
    //delete tcpServer;
    clientSocket->close();

    /*간단한정보를 종료되는 순간 소멸자에서 현재 네임에디트를 저장*/
    QSettings settings("ChatClient", "ChatClient");
    settings.setValue("ChatClient/ID", ui->manager->text());
}

void Chetting::receiveTCPClientName(QString name)
{
    QList<QString> list;
    list.insert(0, name);
}

/*tcp client and protocol*/
void Chetting::closeEvent(QCloseEvent*)
{
    /*1019 코드 수정*/
    sendProtocol(Chat_LogOut, ui->manager->text().toStdString().data());
    clientSocket->disconnectFromHost();
    if(clientSocket->state() != QAbstractSocket::UnconnectedState)
        clientSocket->waitForDisconnected();
}


void Chetting::receiveData( )
{
    QTcpSocket *clientSocket = dynamic_cast<QTcpSocket *>(sender( ));
    if (clientSocket->bytesAvailable( ) > BLOCK_SIZE) return;
    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);

    /*새로 만들기*/
    QString ClientName;
    //char* ch_Name = ClientName;

    StatusOfChat type;      /*채팅의 목적*/

    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in.device()->seek(0);
    in >> type /*>> ClientName*/;

    char data[1020 /*- sizeof(ClientName)*/];        /*전송되는 메시지/데이터*/
    memset(data, 0, 1020 /*- sizeof(ClientName)*/);

    qDebug() << ClientName;
    in.readRawData(data, 1020 /*- sizeof(ClientName)*/);

    switch(type) {
    case Chat_Talk:
        ui->textmessage->append(QString(data));
        ui->inputEdit->setEnabled(true);
        ui->sendButton->setEnabled(true);
        ui->connectButton->setEnabled(true);


        /*file버튼 enable 상태 추가*/
        ui->FileSendButton->setEnabled(true);
        break;
    case Chat_KickOut:
        QMessageBox::critical(this, tr("Chatting Client"), \
                              tr("Kick out from Server"));
        ui->inputEdit->setDisabled(true);
        ui->sendButton->setDisabled(true);
        /*file과 이름 disable상태로 변환*/
        ui->FileSendButton->setDisabled(true);
        ui->manager->setReadOnly(false);
        ui->connectButton->setText("Chat in");
        break;
    case Chat_Invite:
        QMessageBox::information(this, tr("Chatting Client"), \
                                 tr("Invited from Server"));


        /*프로토콜을 보내서 챗인 상태로 전환하는 좋은 방법*/
        sendProtocol(Chat_In, ClientName.toStdString().data());
        ui->inputEdit->setEnabled(true);
        ui->sendButton->setEnabled(true);
        /*파일버튼과 이름 에디터 상태 추가*/
        ui->FileSendButton->setEnabled(true);
        ui->manager->setReadOnly(true);
        ui->connectButton->setText("Chat Out");
        break;
    };
}

void Chetting::sendData(  )
{
    QString str = ui->inputEdit->text( );
    if(str.length( )) {
        QByteArray bytearray;
        bytearray = str.toUtf8( );
        ui->textmessage->append
                (QString("<font color=red>%1</font> : ").
                 arg(ui->manager->text()) + str);
        /*1019코드 수정*/
        sendProtocol(Chat_Talk, bytearray.data());
    }
}

/*1019 코드 수정*/
void Chetting::sendFile( )
{
    //가변수 초기화
    loadSize = 0;
    byteToWrite = 0;
    totalSize = 0;
    outBlock.clear();

    QString filename = QFileDialog::getOpenFileName(this);

    file = new QFile(filename);
    file->open(QFile::ReadOnly);

    qDebug() << QString("file %1 is opened").arg(filename);
    progressDialog->setValue(0);   //파일을 보내지 않으면 처음 상태는 0으로 조정

    if(!isSent)         /*처음 전송될 때만 연결이 신호 연결을 생성할 때 발생합니다.*/
    {
        fileSocket->connectToHost(ui->ipAddress->text(),
                                  ui->portNumber->text().toInt() + 1);
        isSent = true;
    }

    //처음으로 보낼 때 connectToHost는 send를 호출하기 위해 연결 신호를 시작하고 두 번째 후에는 send를 호출해야 합니다.
    byteToWrite = totalSize = file->size(); /*남은 데이터 사이즈를 정의*/
    loadSize = 1024; // 매번 전송되는 데이터의 크기

    QDataStream out(&outBlock, QIODevice::WriteOnly);
    out << qint64(0) << qint64(0) << filename << ui->manager->text();

    totalSize += outBlock.size();
    //전체 크기는 파일 크기에 파일 이름 및 기타 정보의 크기를 더한 것입니다.
    byteToWrite += outBlock.size();

    out.device()->seek(0);
    //바이트 스트림의 시작 부분으로 돌아가 전체 크기와 파일 이름 및 기타 정보 크기인 qint64를 앞에 씁니다.
    out << totalSize << qint64(outBlock.size());

    fileSocket->write(outBlock);/*읽은 파일을 소켓으로 보냅니다.*/

    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(totalSize-byteToWrite);
    progressDialog->show();

    qDebug() << QString("Sending file %1").arg(filename);
}

void Chetting::sendProtocol(StatusOfChat type, char* data, int size)
{
    QByteArray dataArray;           // 소켓으로 보낼 데이터를 채우고
    QDataStream out(&dataArray, QIODevice::WriteOnly);
    out.device()->seek(0);
    out << type;
    out.writeRawData(data, size);
    clientSocket->write(dataArray);     // 서버로 전송
    clientSocket->flush();
    while(clientSocket->waitForBytesWritten());
}

void Chetting::goOnSend(qint64 numBytes) //파일 보내기 시작
{
    byteToWrite -= numBytes; // 남은 데이터의 크기
    outBlock = file->read(qMin(byteToWrite, numBytes));
    fileSocket->write(outBlock);

    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(totalSize-byteToWrite);

    if (byteToWrite == 0) { // 데이터를 모두 보냈을 경우
        qDebug("File sending completed!");
        progressDialog->reset();
    }
}

void Chetting::disconnect( )            /*관리자 채팅 클라이언트의 접속이 단절 되었을 때*/
{
    QMessageBox::critical(this, tr("manager Client"), \
                          tr("Disconnect from Server"));        /*위험 메세지 출력*/
    ui->inputEdit->setEnabled(false);
    ui->manager->setReadOnly(false);
    ui->sendButton->setEnabled(false);
    ui->FileSendButton->setEnabled(false);          /*다시 접속했을 때는 비활성 모드로 변경*/

    /*커넥트 버튼 상태 추가*/
    ui->connectButton->setText(tr("Log In"));       /*로그인이 가능하도록 하는 버튼 활성화*/
    ui->serverstatus->setText("only manager chetting client");
    emit signalClient(0, ui->manager->text().toStdString().data());
    //    close( );
}



