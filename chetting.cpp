#include "chetting.h"
#include "ui_chetting.h"
#include <QtNetwork>
#include <QMessageBox>
#include <QDataStream>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QDateTime>


#define BLOCK_SIZE 1024

Chetting::Chetting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chetting)
{
    ui->setupUi(this);

    /*tcp_server*/
    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::Any, 8001);
    connect(tcpServer, SIGNAL(newConnection()), SLOT(clientConnect()));
    //    if(!tcpServer->listen())
    //    {
    //        QMessageBox::critical(this, tr("Echo Server"),
    //                              tr("Unable to start the server: %1.")\
    //                              .arg(tcpServer->errorString()));
    //        close();
    //        return;
    //    }

    ui->serverstatus->setText(tr("The server is running on port %1.")
                              .arg(tcpServer->serverPort()));



    /*tcp client*/
    ui->textmessage->setReadOnly(true);
    ui->ipAddress->setText("127.0.0.1");
    QRegularExpression re("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$|");

    QRegularExpressionValidator validator(re);
    ui->ipAddress->setPlaceholderText("Server IP Address");
    ui->ipAddress->setValidator(&validator);


    ui->portNumber->setText("8000");
    ui->portNumber->setInputMask("00000;_");
    ui->portNumber->setPlaceholderText("Server Port No");

    /*소켓도 항시 초기화가 필요함*/
    clientSocket = new QTcpSocket(this);

    /*연결할 호스트 서버 연결*/
    connect(ui->connectButton,
            &QPushButton::clicked,
            [=]{
        if(ui->connectButton->text() == tr("connect"))
        {
            clientSocket->connectToHost(
                        ui->ipAddress->text(),
                        ui->portNumber->text().toInt());
            connectToServer();      //서버 커넥트
            ui->connectButton->setText("Chat in");
        }
        else
        {
            chattingProtocol data;  //구조체를 채우고
            data.type = Chat_In;        /*Chat_In으로 타입 전환*/
            qstrcpy(data.data, ui->IPNameEdit->text().toStdString().data());/*이름 찾기*/


            QByteArray sendArray;       /*소켓으로 보낼 데이터를 채움*/
            QDataStream out(&sendArray, QIODevice::WriteOnly);
            out << data.type;
            out.writeRawData(data.data, 1020);
            clientSocket->write(sendArray);
        }
        ui->connectButton->setDisabled(true);
        ui->inputEdit->setEnabled(true);
        ui->sendButton->setEnabled(true);
    });

    /*서버로 보낼 메시지를 위한 위젯들*/
    connect(ui->inputEdit, SIGNAL(returnPressed( )), SLOT(sendData( )));
    connect(ui->inputEdit, SIGNAL(returnPressed( )), ui->inputEdit, SLOT(clear( )));

    connect(ui->sendButton, SIGNAL(clicked( )), SLOT(sendData( )));
    connect(ui->sendButton, SIGNAL(clicked( )), ui->inputEdit, SLOT(clear( )));
    ui->inputEdit->setEnabled(false);
    ui->sendButton->setEnabled(false);

    /*connect 버튼을 누를시 이름이 받아짐*/
//        connect(ui->connectButton, SIGNAL(clicked()),
//                this, SLOT(receiveTCPClientName(QString)));
    /*위 주석은 이미 메인 윈도우에 커텍트 되어 따로 연동할 필요 없음*/
    /*단 밑의 커넥트는 관리자로 로그인 하기 위한 커넥트*/
    connect(ui->connectButton,
            &QPushButton::clicked,
            [=]{
        QString name;
        QList<QString> list;
        name = ui->IPNameEdit->text();
        list.insert(0, name);
        ui->listWidget->addItems(list);
    });     /*관리지기 로그인 하기 위해 커넥트*/


    /*reduce button을 누를 시 해당 list원소를 삭제*/
    connect(ui->reduceButton,
            &QPushButton::clicked,
            [=]{
        /*리스트 안에 해당되는 아이템을 선택후 제거하는 코드*/
        QList<QListWidgetItem*> items = ui->listWidget->selectedItems();

        foreach(QListWidgetItem *item, items)
        { delete ui->listWidget->takeItem(ui->listWidget->row(item));}

        /*이제 여기서 disconnect만 구현하면 추방기능은 만들어지는 것 같음*/
        chattingProtocol data;
        data.type = Chat_KickOut;
        qstrcpy(data.data, ui->reduceEdit->text().toStdString().data());

        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << data.type;
        out.writeRawData(data.data, 1020);
        clientSocket->write(sendArray);

        clientSocket->disconnectFromHost();
        while(clientSocket->waitForDisconnected(1000))
            QThread::usleep(10);

        /*제거가 완료되면 해당 채팅 서버 이름 메시지 박스 출력*/
        if(ui->reduceEdit->text() != nullptr)
        {
            QString name = ui->reduceEdit->text().toStdString().data();
            QMessageBox::critical(this, tr(" Chatting Client"),
                        QString("%1\n Disconnect from Server").arg(name));
        }


        /*제거가 완료되면 텍스트를 공백으로 만듦*/
        ui->reduceEdit->setText("");

    });

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

    ui->IPNameEdit->setText("No names");
}

/*tcp server*/
void Chetting::echoData()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));
    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);
    foreach(QTcpSocket *sock, serverList) {
        if(sock != clientConnection)
            sock->write(bytearray);
    }
    ui->serverstatus->setText(QString(bytearray));

}

void Chetting::receiveTCPClientName(/*int id,*/ QString name)
{
    QList<QString> list;
    //id = ui->managerEdit->text().toInt();
    name = ui->IPNameEdit->text();
    list.insert(0, name);
    ui->listWidget->addItems(list);
}

void Chetting::receiveClient(/*int id,*/QString name)
{
//    QString ID;
//    ID = ID.setNum(id);
//    ui->managerEdit->setText(ID);
    ui->IPNameEdit->setText(name);
}

void Chetting::clientConnect()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection( );
    connect(clientConnection, SIGNAL(disconnected( )), \
            clientConnection, SLOT(deleteLater( )));
    connect(clientConnection, SIGNAL(readyRead( )), SLOT(echoData( )));
    connect(clientConnection, SIGNAL(disconnected()), SLOT(removeItem()));
    ui->serverstatus->setText("new connection is established...");

    serverList.append(clientConnection);        // QList<QTcpSocket*> clientList;
}

void Chetting::removeItem()
{
    QTcpSocket* clientConnection = dynamic_cast<QTcpSocket*>(sender());
    serverList.removeOne(clientConnection);
    clientConnection->deleteLater();
}

/*tcp client and protocol*/
void Chetting::closeEvent(QCloseEvent*)
{
    chattingProtocol data;
    data.type = Chat_LogOut;
    qstrcpy(data.data, ui->IPNameEdit->text().toStdString().data());

    QByteArray sendArray;
    QDataStream out(&sendArray, QIODevice::WriteOnly);
    out << data.type;
    out.writeRawData(data.data, 1020);
    clientSocket->write(sendArray);

    clientSocket->disconnectFromHost();
    while(clientSocket->waitForDisconnected(1000))
        QThread::usleep(10);
}

void Chetting::receiveData( )
{
    QTcpSocket *clientSocket = dynamic_cast<QTcpSocket *>(sender( ));
    if (clientSocket->bytesAvailable( ) > BLOCK_SIZE) return;
    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);
    chattingProtocol data;
    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in >> data.type;
    in.readRawData(data.data, 1020);

    qDebug( ) << data.type;
    switch(data.type) {
    case Chat_Talk:
        ui->textmessage->append(QString(data.data));
        break;
        ui->inputEdit->setEnabled(true);
        ui->sendButton->setEnabled(true);
        ui->connectButton->setDisabled(true);
    case Chat_KickOut:
        QMessageBox::critical(this, tr("Chatting Client"), \
                              tr("Kick out from Server"));
        ui->inputEdit->setEnabled(false);
        ui->sendButton->setEnabled(false);
        ui->connectButton->setEnabled(true);
        break;
    case Chat_Invite:
        QMessageBox::information(this, tr("Chatting Client"), \
                              tr("Invited from Server"));
        ui->inputEdit->setEnabled(true);
        ui->sendButton->setEnabled(true);
        ui->connectButton->setDisabled(true);
        break;
    };
}

void Chetting::sendData(  )
{
    QString str = ui->inputEdit->text( );
    if(str.length( )) {
        QByteArray bytearray;
        bytearray = str.toUtf8( );
        ui->textmessage->append("<font color=red>나</font> : " + str);

        chattingProtocol data;
        data.type = Chat_Talk;
        qstrcpy(data.data, bytearray.data());

        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << data.type;
        out.writeRawData(data.data, 1020);
        clientSocket->write(sendArray);
    }
}

void Chetting::connectToServer( )
{
    chattingProtocol data;
    data.type = Chat_Login;
    qstrcpy(data.data, ui->IPNameEdit->text().toStdString().data());

    QByteArray sendArray;
    QDataStream out(&sendArray, QIODevice::WriteOnly);
    out << data.type;
    out.writeRawData(data.data, 1020);
    clientSocket->write(sendArray);
}

void Chetting::disconnect( )
{
    QMessageBox::critical(this, tr("Chatting Client"), \
                          tr("Disconnect from Server"));
    ui->inputEdit->setEnabled(false);
    ui->sendButton->setEnabled(false);
    //    close( );
}

//void Chetting::CReceiveData(QString str)
//{
//    ui->IPNameEdit->setText(str);
//}

Chetting::~Chetting()
{
    delete ui;
    delete tcpServer;
    clientSocket->close();

    /*간단한정보를 종료되는 순간 소멸자에서 현재 네임에디트를 저장*/
    QSettings settings("ChatClient", "ChatClient");
    settings.setValue("ChatClient/ID", ui->IPNameEdit->text());
}

void Chetting::on_listWidget_itemClicked(QListWidgetItem *item)
{
    ui->reduceEdit->setText(item->text());
}

