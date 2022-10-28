#include "tcpserver.h".h"
#include "chetting.h"
#include "ui_tcplog.h"
#include "logthread.h"

#include <QPushButton>
#include <QBoxLayout>
#include <QTcpServer>
#include <QTcpSocket>
#include <QApplication>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QDebug>
#include <QMenu>
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>

TCPServer::TCPServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tcplog), totalSize(0), byteReceived(0)   /*데이터 가변수 0으로 초기화*/
{
    ui->setupUi(this);
    QList<int> sizes;
    sizes << 200 << 400;
    ui->splitter->setSizes(sizes);

    /*채팅을 위한 서버 할당*/
    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection( )), SLOT(clientConnect( )));
    /*newConnection : 이 신호는 새 연결을 사용할 수 있을 때마다 발생합니다.*/
    /*clientConnect 채팅을 위한 슬롯 커넥트*/
    if (!tcpServer->listen(QHostAddress::Any, PORT_NUMBER)) {       /*PORT NUMBER = 8000*/
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(tcpServer->errorString( )));
        close( );
        return;
    }

    /*파일 선송을 위한 서버*/
    fileServer = new QTcpServer(this);
    connect(fileServer, SIGNAL(newConnection()), SLOT(acceptConnection()));
    /*accpetConnection 파일 전송을 위한 커넥트*/
    if (!fileServer->listen(QHostAddress::Any, PORT_NUMBER+1)) {    /*PORT NUMBER = 8001*/
        QMessageBox::critical(this, tr("Chatting Server"),         /*채팅의 과 파일 전송의 포트가 같으면*/
                              tr("Unable to start the server: %1.") /*통신이 안될 수 있으므로 다른포트로 저장*/
                              .arg(fileServer->errorString( )));
        close( );
        return;
    }

    qDebug("Start listening ...");

    /*초대 액션 수행 할당*/
    QAction* inviteAction = new QAction(tr("&Invite"));
    inviteAction->setObjectName("Invite");
    connect(inviteAction, SIGNAL(triggered()), SLOT(inviteClient()));

    /*제거 액션 수행 할당*/
    QAction* removeAction = new QAction(tr("&Kick out"));
    connect(removeAction, SIGNAL(triggered()), SLOT(kickOut()));

    menu = new QMenu;
    menu->addAction(inviteAction);  /*초대 액션 추가*/
    menu->addAction(removeAction);  /*강제 퇴장 액션 추가*/
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    /*컨텍스트 메뉴 정의 및 세팅*/

    /*파일 전송 상태를 출력하기 위한 프로그래스바 출력*/
    progressDialog = new QProgressDialog(0);
    /*최소 사이즈의 프로그래스 창 출력*/
    progressDialog->setAutoClose(true);     /*자동 닫기 설정*/
    progressDialog->reset();                /*리셋*/

    /*채팅 기록을 1분마다 저장할 수 있는 로그 스레드 생성후 스레드 동작*/
    logThread = new LogThread(this);
    logThread->start();

    connect(ui->saveButton, SIGNAL(clicked()),
            logThread, SLOT(appendData(QTreeWidgetItem*)));

    qDebug() << tr("The server is running on port %1.").arg(tcpServer->serverPort( ));

    connect(ui->saveButton, SIGNAL(clicked()), logThread, SLOT(saveData()));
}

TCPServer::~TCPServer()
{
    delete ui;

    logThread->terminate();     /*로그 스레드 종료*/
    tcpServer->close( );        /*채팅 서버 종료*/
    fileServer->close( );       /*파일 서버 종료*/
}

void TCPServer::clientConnect( )
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection( );
    connect(clientConnection, SIGNAL(readyRead( )), SLOT(receiveData( )));
    connect(clientConnection, SIGNAL(disconnected( )), SLOT(removeClient()));
    qDebug("new connection is established...");
}

void TCPServer::receiveData()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( )); //해당 소켓의 시그널을 발생시킬 객체의 포인터 지정
    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);

    Chat_Status type;       // 채팅의 목적
    char data[1020];        // 전송되는 메시지/데이터
    memset(data, 0, 1020);  // data총량 중 1020바이트를 모두 0으로 초기화

    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in.device()->seek(0);
    in >> type;
    in.readRawData(data, 1020);

    QString ip = clientConnection->peerAddress().toString();   /*ip주소 할당*/
    quint64 port = clientConnection->peerPort();            /*포트 번호 할당*/
    QString name = QString::fromStdString(data);

    qDebug() << ip << " : " << type;

    switch(type) {
    case Server_Chat_Login:         /*채팅방에서 로그인 버튼을 클릭시 로그인 상태로 전환되는 경우*/
        foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            if(item->text(0) != "-") {             /*대기 중이 아닌 경우는 X밖에 없음*/
                item->setText(0, "-");              /*대기 상태 표시*/
                clientList.append(clientConnection);        // QList<QTcpSocket*> clientList;
                clientSocketHash[name] = clientConnection;
            }
        }
        break;
    case Server_Chat_In:
        foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            if(item->text(0) != "O") {
                item->setText(0, "O");
            }
            clientNameHash[port] = name;
        }
        break;
    case Server_Chat_Talk: {         /*클라이언트 채팅간 보내는 이의 이름과 채팅내용을 기록*/
        foreach(QTcpSocket *sock, clientList) {
            if(clientNameHash.contains(sock->peerPort()) && sock != clientConnection) {
                QByteArray sendArray;
                sendArray.clear();
                QDataStream out(&sendArray, QIODevice::WriteOnly);
                out << Chat_Talk;
                sendArray.append("<font color=lightsteelblue>");
                sendArray.append(clientNameHash[port].toStdString().data());
                sendArray.append("</font> : ");
                sendArray.append(name.toStdString().data());
                sock->write(sendArray);
                qDebug() << sock->peerPort();
            }
        }

        /*채팅을 보낼때마다 로그를 기록*/

        CName = clientNameHash[port];
        CId = (clientIDHash[clientNameHash[port]]);
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->logtreeWidget);
        item->setText(0, ip);                               /*IP주소*/
        item->setText(1, QString::number(port));            /*포트 번호*/
        item->setText(2, /*QString::number(clientIDHash[name])*/    /*고객 id*/
                      QString::number(CId));
        item->setText(3, CName);                            /*해당 포트의 주인 이름(고객 성함)*/
        item->setText(4, QString(data));                    /*메세지 데이터*/
        item->setText(5, QDateTime::currentDateTime().toString());  /*로그 등록 시간*/
        item->setToolTip(4, QString(data));                 /*메세지의 커서를 가져다 주면 조그마한 창으로 메세지 출력*/
        ui->logtreeWidget->addTopLevelItem(item);           /*로그 추가*/

        for(int i = 0; i < ui->logtreeWidget->columnCount(); i++)
            ui->logtreeWidget->resizeColumnToContents(i);

        logThread->appendData(item);
    }
        break;
    case Server_Chat_Close:
        foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchContains, 1)) {
            if(item->text(0) != "-") {
                item->setText(0, "-");
            }
            clientNameHash.remove(port);
        }
        break;
    case Chat_LogOut:
        foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchContains, 1)) {
            if(item->text(0) != "X") {
                item->setText(0, "X");
                clientList.removeOne(clientConnection);        // QList<QTcpSocket*> clientList;
                clientSocketHash.remove(name);
            }
        }
        break;
    }
}

void TCPServer::removeClient()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));

    QString name = clientNameHash[clientConnection->peerPort()];
    foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchContains, 1)) {
        item->setText(0, "X");
    }

    clientList.removeOne(clientConnection);
    clientConnection->deleteLater();
}

/*고객의 정보를 추가 시 서버의 고객 리스트의 이름이 추가되는 슬롯*/
void TCPServer::addClient(int id, QString name)
{
    clientIDList << id;
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);    /*클라이언트 리스트에 있는 아이템 클래스 변수 선언*/
    item->setText(0, "X");                      /*추가된 아이디의 현상태는 X*/
    item->setText(1, name);                     /*해당되는 이름으로 리스트에 추가*/
    ui->treeWidget->addTopLevelItem(item);      /*해당되는 아이템 추가*/
    clientIDHash[name] = id;                    /*이름이 위치한 아이디 할당*/
    ui->treeWidget->resizeColumnToContents(0);  /*내용의 크기에 맞게 열의 크기를 조정합니다.*/
}

/*고객의 정보를 제거 시 서버의 고객 리스트의 이름이 제거 되는 슬롯*/
void TCPServer::removeClient(int id)
{
    qDebug() << id;
    ui->treeWidget->takeTopLevelItem(id);       /*해당되는 아이템을 리스트에서 제거*/
}


/*고객의 정보를 수정 시 서버의 고객 리스트의 이름이 수정되는 슬롯*/
void TCPServer::modifyClient(int id, QString name, int index)
{
    ui->treeWidget->takeTopLevelItem(index);
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);    /*할당된 아이템 삭제후*/
    item->setText(0, "X");                              /*새로이 수정된 아이템을 추가*/
    item->setText(1, name);
    ui->treeWidget->addTopLevelItem(item);
    clientIDHash[name] = id;
    ui->treeWidget->resizeColumnToContents(0);

    qDebug() << clientIDHash;
}

void TCPServer::receiveManager(int id, QString name)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, "X");
    item->setText(1, name);
    ui->treeWidget->addTopLevelItem(item);
    clientIDHash[name] = id;
    ui->treeWidget->resizeColumnToContents(0);
    item->setHidden(true);
}


void TCPServer::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->treeWidget->currentItem())               /*빈 커렌트 아이템 클릭시 예외처리 완료*/
    {
        foreach(QAction *action, menu->actions()) {
            if(action->objectName() == "Invite")
                /*Invite라는 컨택스트 메뉴가 '-'상태일 때 활성화*/
                action->setEnabled(ui->treeWidget->currentItem()->text(0) == "-");
            else
                /*Kick out 이라는 컨택스트 메뉴가 'O'상태일 때 활성화*/
                action->setEnabled(ui->treeWidget->currentItem()->text(0) == "O");
        }
        QPoint globalPos = ui->treeWidget->mapToGlobal(pos);
        menu->exec(globalPos);
    }
    else
    {
        return;
    }
}
void TCPServer::CheckLogIn(QString name)
{
    auto flag = Qt::MatchFixedString;
    int Check = 1;

    if(ui->treeWidget->findItems(name, flag, 1).count() == 0)
        /*고객 리스트에 해당되는 이름이 있는경우*/
    {
        // 찾는 고객명의 데이터가 없을 때
        Check = 0;
        emit SendLogInChecked(Check);
    }
    // 찾는 고객명의 데이터가 있을 때
    else
        emit SendLogInChecked(Check);

}

/*서버에서 해당하는 고객을 강제 퇴장 할 시*/
void TCPServer::kickOut()
{
    QString name = ui->treeWidget->currentItem()->text(1);          /*이름으로 데이터를 식볗*/
    QTcpSocket* sock = clientSocketHash[name];              /*해당이름을 키값으로 소켓을 보냄*/

    QByteArray sendArray;
    QDataStream out(&sendArray, QIODevice::WriteOnly);
    out << Chat_KickOut;                /*현 서버의 타입은 강제 퇴장*/
    out.writeRawData("", 1020);
    sock->write(sendArray);

    ui->treeWidget->currentItem()->setText(0, "-");     /*퇴장시 대기실에 머뭄*/
}

void TCPServer::inviteClient()
{
    if(ui->treeWidget->topLevelItemCount()) {
        QString name = ui->treeWidget->currentItem()->text(1);

        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << Chat_Invite /*<< name*/;

        out.writeRawData("", 1020 /*- sizeof(name)*/);  /*이름의 크기 만큼 원래 채팅 데이터에서 빼줘야지 신호 억세*/
        QTcpSocket* sock = clientSocketHash[name];

        if(sock != nullptr)     /*socket의 정보가 있는경우에만 인바이트 활성화*/
        {
            sock->write(sendArray);

            foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchFixedString, 1)) {
                if(item->text(0) != "O") {
                    item->setText(0, "O");
                }
            }

            quint64 port = sock->peerPort();
            clientNameHash[port] = name;
        }

    }
}

/* 파일 전송 */
void TCPServer::acceptConnection()
{
    qDebug("Connected, preparing to receive files!");

    QTcpSocket* receivedSocket = fileServer->nextPendingConnection();
    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

void TCPServer::readClient()    /*들어온 파일을 프로젝트가 읽을 수 있게끔 하는 코드*/
{
    qDebug("Receiving file ...");
    QTcpSocket* receivedSocket = dynamic_cast<QTcpSocket *>(sender( ));
    QString filename;
    QString name;

    if (byteReceived == 0) { // 방금 데이터 수신을 시작, 이 데이터는 파일 정보입니다.
        progressDialog->reset();    /*프로그래스 바 리셋*/
        progressDialog->show();     /*다이얼로그 출력*/

        QString ip = receivedSocket->peerAddress().toString();
        quint16 port = receivedSocket->peerPort();

        QDataStream in(receivedSocket);         /*데이터의 흐름을 읽는 변수 선언*/
        in >> totalSize >> byteReceived >> filename >> name;    /*1.파일의 전체 사이즈, 2.읽어드리는 데이터, 3.파일경로, 4.파일이름*/
        progressDialog->setMaximum(totalSize);              /*프로그래스 바는 전체사이즈로 설정한 가변수의 길이 만큼 지정*/

        /*서버에서 저장되는 로그를 기록*/
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->logtreeWidget);
        item->setText(0, ip);
        item->setText(1, QString::number(port));
        item->setText(2, QString::number(clientIDHash[name]));
        item->setText(3, name);
        item->setText(4, filename);
        item->setText(5, QDateTime::currentDateTime().toString());
        item->setToolTip(4, filename);      /*해당 마우스 커서를 가져다 두었을때 조그만 창으로 출력*/
        for(int i = 0; i < ui->logtreeWidget->columnCount(); i++)   /*열의 개수 할당*/
            ui->logtreeWidget->resizeColumnToContents(i);       /*내용의 크기에 맞게 열의 크기를 조정*/

        /*파일(텍스트, 이미지)을 보낼 때마다 로그에 저장*/
        ui->logtreeWidget->addTopLevelItem(item);

        /*스레드도 입력되는 아이템마다 기록*/
        logThread->appendData(item);

        QFileInfo info(filename);
        QString currentFileName = info.fileName();
        file = new QFile(currentFileName);
        file->open(QFile::WriteOnly);
    } else { // Officially read the file content
        inBlock = receivedSocket->readAll();

        /*읽어드린 데이터를 byteReceived변수로 증가연산*/
        byteReceived += inBlock.size();
        file->write(inBlock);
        file->flush();
    }

    progressDialog->setValue(byteReceived);

    /*파일 전송 완료시*/
    if (byteReceived == totalSize) {
        qDebug() << QString("%1 receive completed").arg(filename);

        inBlock.clear();
        byteReceived = 0;           /*받아진 데이터 표시 0*/
        totalSize = 0;              /*파일 전체 크기 0*/
        progressDialog->reset();    /*프로그래스 바 리셋*/
        progressDialog->hide();     /*다이얼로그는 숨겨진 상태*/
        file->close();              /*open되었던 파일은 다시 닫아주기*/
        delete file;
        /*보내는 데이터를 초기화*/
    }
}
