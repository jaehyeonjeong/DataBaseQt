#include "tcpserver.h"
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

/*1019 강사님께서 작성하신 코드*/
#include <QFile>                /*파일 해더*/
#include <QFileInfo>            /*파일 정보  헤더*/
#include <QProgressDialog>      /*프로그래그 다이얼로그 해더*/

TCPServer::TCPServer(QWidget *parent) :
    QWidget(parent), ui(new Ui::tcplog), totalSize(0), /*데이터와 리시브 초기화*/
    byteReceived(0)
{
    ui->setupUi(this);
    QList<int> sizes;
    sizes << 120 << 500;
    ui->splitter->setSizes(sizes);

    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection( )), SLOT(clientConnect( )));
    if (!tcpServer->listen(QHostAddress::Any, PORT_NUMBER)) {
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(tcpServer->errorString( )));
        close( );
        return;
    }

    fileServer = new QTcpServer(this);
    connect(fileServer, SIGNAL(newConnection()), SLOT(acceptConnection()));
    if (!fileServer->listen(QHostAddress::Any, PORT_NUMBER+1)) {
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(fileServer->errorString( )));
        close( );
        return;
    }

    qDebug("Start listening ...");

    QAction* inviteAction = new QAction(tr("&Invite"));
    inviteAction->setObjectName("Invite");
    connect(inviteAction, SIGNAL(triggered()), SLOT(inviteClient()));

    QAction* removeAction = new QAction(tr("&Kick out"));
    connect(removeAction, SIGNAL(triggered()), SLOT(kickOut()));

    menu = new QMenu;
    menu->addAction(inviteAction);
    menu->addAction(removeAction);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();

    logThread = new LogThread(this);
    logThread->start();

    qDebug() << tr("The server is running on port %1.").arg(tcpServer->serverPort( ));
}

TCPServer::~TCPServer()
{
    delete ui;
//    delete menu;
    /*쓰레드, 채팅, 파일 모드 종료*/
    logThread->terminate();
    fileServer->close();
    tcpServer->close( );

}

void TCPServer::clientConnect( )
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection( );
    connect(clientConnection, SIGNAL(readyRead( )), SLOT(receiveData( )));
    connect(clientConnection, SIGNAL(disconnected( )), SLOT(removeClient()));
    qDebug("new connection is established...");
}

void TCPServer::receiveData( )
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));
    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);

    Chat_Status type;       // 채팅의 목적
    char data[1020];        // 전송되는 메시지/데이터
    memset(data, 0, 1020);

    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in.device()->seek(0);
    in >> type;
    in.readRawData(data, 1020);

    QString ip = clientConnection->peerAddress().toString();
    quint16 port = clientConnection->peerPort();
    QString name = QString::fromStdString(data);

    qDebug() << ip << " : " << type;

    switch(type) {
    case Server_Chat_Login:
        foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            if(item->text(0) != "-") {
                item->setText(0, "-");
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
    case Server_Chat_Talk: {
        foreach(QTcpSocket *sock, clientList) {
            if(clientNameHash.contains(sock->peerPort()) && sock != clientConnection) {
                QByteArray sendArray;
                sendArray.clear();
                QDataStream out(&sendArray, QIODevice::WriteOnly);
                out << Server_Chat_Talk;
                sendArray.append("<font color=lightsteelblue>");
                sendArray.append(clientNameHash[port].toStdString().data());
                sendArray.append("</font> : ");
                sendArray.append(name.toStdString().data());
                sock->write(sendArray);
                qDebug() << sock->peerPort();
            }
        }

        QTreeWidgetItem* item = new QTreeWidgetItem(ui->logtreeWidget);
        item->setText(0, ip);
        item->setText(1, QString::number(port));
        item->setText(2, QString::number(clientIDHash[clientNameHash[port]]));
        item->setText(3, clientNameHash[port]);

//        item->setText(2, QString::number(clientIDHash[oldClientNameHash[ip]]));
//        item->setText(3, oldClientNameHash[ip]);

        item->setText(4, QString(data));
        item->setText(5, QDateTime::currentDateTime().toString());
        item->setToolTip(4, QString(data));

        for(int i = 0; i < ui->logtreeWidget->columnCount(); i++)
            ui->logtreeWidget->resizeColumnToContents(i);

        ui->logtreeWidget->addTopLevelItem(item);

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
    case Server_Chat_LogOut:
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
    clientList.removeOne(clientConnection);
    clientConnection->deleteLater();

    QString name = clientNameHash[clientConnection->peerPort()];
    foreach(auto item, ui->logtreeWidget->findItems(name, Qt::MatchContains, 1)) {
        //item->setText(0, "X");
    }
}

void TCPServer::addClient(int id, QString name)
{
    clientIDList << id;
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, "X");
    item->setText(1, name);
    ui->treeWidget->addTopLevelItem(item);
    clientIDHash[name] = id;
    ui->treeWidget->resizeColumnToContents(0);
}

void TCPServer::receiveManager(int id, QString name)
{
    clientIDList << id;
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, "X");
    item->setText(1, name);
    ui->treeWidget->addTopLevelItem(item);
    clientIDHash[name] = id;
    ui->treeWidget->resizeColumnToContents(0);
}

void TCPServer::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    foreach(QAction *action, menu->actions()) {
        if(action->objectName() == "Invite")
            action->setEnabled(ui->treeWidget->currentItem()->text(0) != "O");
        else
            action->setEnabled(ui->treeWidget->currentItem()->text(0) == "O");
    }
    QPoint globalPos = ui->treeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void TCPServer::kickOut()
{
    QString name = ui->treeWidget->currentItem()->text(1);
    QTcpSocket* sock = clientSocketHash[name];

    QByteArray sendArray;
    QDataStream out(&sendArray, QIODevice::WriteOnly);
    out << Server_Chat_KickOut;
    out.writeRawData("", 1020);

//            sock->disconnectFromHost();
    sock->write(sendArray);
    ui->treeWidget->currentItem()->setText(0, "-");
//    clientIDList.append(clientIDHash[name]);
//    ui->inviteComboBox->addItem(name);
}

void TCPServer::inviteClient()
{
    if(ui->treeWidget->topLevelItemCount()) {
        QString name = ui->treeWidget->currentItem()->text(1);

        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << Server_Chat_Invite;
        out.writeRawData("", 1020);
        QTcpSocket* sock = clientSocketHash[name];

        sock->write(sendArray);
        foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            if(item->text(1) != "O") {
                item->setText(0, "O");
//                clientList.append(sock);        // QList<QTcpSocket*> clientList;
            }
        }
    }
}

void TCPServer::acceptConnection()
{
    qDebug("Connected, preparing to receive files!");

    QTcpSocket* receivedSocket = fileServer->nextPendingConnection();
    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

void TCPServer::readClient()
{
    qDebug("Receiving file ...");
    QTcpSocket* receivedSocket = dynamic_cast<QTcpSocket *>(sender( ));
    QString filename;

    if (byteReceived == 0) { // just started to receive data, this data is file information
        progressDialog->reset();
        progressDialog->show();

        QString ip = receivedSocket->peerAddress().toString();
        quint16 port = receivedSocket->peerPort();

        QTreeWidgetItem* item = new QTreeWidgetItem(ui->logtreeWidget);
        item->setText(0, ip);
        item->setText(1, QString::number(port));
        item->setText(2, QString::number(clientIDHash[clientNameHash[port]]));
        item->setText(3, clientNameHash[port]);
        item->setText(4, filename);
        item->setText(5, QDateTime::currentDateTime().toString());
        item->setToolTip(4, filename);

        for(int i = 0; i < ui->logtreeWidget->columnCount(); i++)
            ui->logtreeWidget->resizeColumnToContents(i);

        ui->logtreeWidget->addTopLevelItem(item);

        logThread->appendData(item);

        QDataStream in(receivedSocket);
        in >> totalSize >> byteReceived >> filename;
        progressDialog->setMaximum(totalSize);

        QFileInfo info(filename);
        QString currentFileName = info.fileName();
        file = new QFile(currentFileName);
        file->open(QFile::WriteOnly);
    } else { // Officially read the file content
        inBlock = receivedSocket->readAll();

        byteReceived += inBlock.size();
        file->write(inBlock);
        file->flush();
    }

    progressDialog->setValue(byteReceived);

    if (byteReceived == totalSize) {
        qDebug() << QString("%1 receive completed").arg(filename);

        inBlock.clear();
        byteReceived = 0;
        totalSize = 0;
        progressDialog->reset();
        progressDialog->hide();
        file->close();
        delete file;
    }
}

