#include "tcpserver.h"
#include "ui_tcplog.h"

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

TCPServer::TCPServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tcplog)
{
    ui->setupUi(this);
    QList<int> sizes;
    sizes << 120 << 500;
    ui->splitter->setSizes(sizes);
    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection( )), SLOT(clientConnect( )));
    if (!tcpServer->listen(QHostAddress::Any, 8000)) {
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(tcpServer->errorString( )));
        close( );
        return;
    }

    /*명단에서 추가해야 할 액션*/
    QAction* inviteAction = new QAction(tr("&Invite"));
    inviteAction->setObjectName("Invite");
    connect(inviteAction, SIGNAL(triggered()), SLOT(inviteClient()));

    QAction* removeAction = new QAction(tr("&Kick out"));
    connect(removeAction, SIGNAL(triggered()), SLOT(kickOut()));

    menu = new QMenu;
    menu->addAction(inviteAction);
    menu->addAction(removeAction);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    /*명단에서 추가해야 할 액션*/

    qDebug() << tr("The server is running on port %1.").arg(tcpServer->serverPort( ));
}

TCPServer::~TCPServer()
{
    delete ui;
    delete menu;

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

    chatProtocol data;
    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in >> data.type;
    in.readRawData(data.data, 1020);

    QString ip = clientConnection->peerAddress().toString();
    QString name = QString::fromStdString(data.data);

    switch(data.type) {
    case Server_Chat_Login:
        foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            if(item->text(1) != "O") {
                item->setText(0, "O");
                clientList.append(clientConnection);        // QList<QTcpSocket*> clientList;
                clientNameHash[ip] = name;
            }
        }
        break;
    case Server_Chat_In:
        foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            if(item->text(1) != "O") {
                item->setText(0, "O");
            }
        }
        break;
    case Server_Chat_Talk: {
        foreach(QTcpSocket *sock, clientList) {
            if(sock != clientConnection) {
                QByteArray data("<font color=red>");
                data.append(name.toStdString().data());
                data.append("</font> : ");
                data.append(bytearray);
                sock->write(data);
            }
        }

        QTreeWidgetItem* item = new QTreeWidgetItem(ui->logtreeWidget);
        item->setText(0, clientConnection->peerAddress().toString());
        item->setText(1, QString::number(clientConnection->peerPort()));
        item->setText(2, QString::number(clientIDHash[clientNameHash[ip]]));
        item->setText(3, clientNameHash[ip]);
        item->setText(4, QString(data.data));
        item->setText(5, QDateTime::currentDateTime().toString());
        item->setToolTip(4, QString(data.data));

        for(int i = 0; i < ui->logtreeWidget->columnCount(); i++)
            ui->logtreeWidget->resizeColumnToContents(i);

        ui->logtreeWidget->addTopLevelItem(item);
    }
        break;
    case Server_Chat_Close:
        foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchContains, 1)) {
            if(item->text(0) == "O") {
                item->setText(0, "X");
            }
        }
        break;
    case Server_Chat_LogOut:
        foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchContains, 1)) {
            if(item->text(0) == "O") {
                item->setText(0, "X");
                clientList.removeOne(clientConnection);        // QList<QTcpSocket*> clientList;
            }
        }
//        ui->inviteComboBox->addItem(name);
        break;
    }
    //    qDebug() << bytearray;
}

void TCPServer::removeClient()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));
    clientList.removeOne(clientConnection);
    clientConnection->deleteLater();

    QString name = clientNameHash[clientConnection->peerAddress().toString()];
    foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchContains, 1)) {
        item->setText(0, "X");
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

void TCPServer::removeClient(int id)
{
    qDebug() << id;
    ui->treeWidget->takeTopLevelItem(id);
}

void TCPServer::modifyClient(QString name, int index)
{
    ui->treeWidget->takeTopLevelItem(index);
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, "X");
    item->setText(1, name);
    ui->treeWidget->addTopLevelItem(item);
    //clientIDHash[name] = id;
    ui->treeWidget->resizeColumnToContents(0);
}

void TCPServer::receiveManager(int id, QString name)
{
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
    QString ip = clientNameHash.key(name);

    chatProtocol data;
    data.type = Server_Chat_KickOut;
    qstrcpy(data.data, "");
    QByteArray sendArray;
    QDataStream out(&sendArray, QIODevice::WriteOnly);
    out << data.type;
    out.writeRawData(data.data, 1020);

    foreach(QTcpSocket* sock, clientList) {
        if(sock->peerAddress().toString() == ip){
//            sock->disconnectFromHost();
            sock->write(sendArray);
        }
    }
    ui->treeWidget->currentItem()->setText(0, "X");
//    clientIDList.append(clientIDHash[name]);
//    ui->inviteComboBox->addItem(name);
}

void TCPServer::inviteClient()
{
    if(ui->treeWidget->topLevelItemCount()) {
        QString name = ui->treeWidget->currentItem()->text(1);
        QString ip = clientNameHash.key(name, "");

        chatProtocol data;
        data.type = Server_Chat_Invite;
        qstrcpy(data.data, "");
        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << data.type;
        out.writeRawData(data.data, 1020);

        foreach(QTcpSocket* sock, clientList) {
            if(sock->peerAddress().toString() == ip){
                sock->write(sendArray);
//                clientList.append(sock);        // QList<QTcpSocket*> clientList;
                foreach(auto item, ui->treeWidget->findItems(name, Qt::MatchFixedString, 1)) {
                    if(item->text(1) != "O") {
                        item->setText(0, "O");
                        clientList.append(sock);        // QList<QTcpSocket*> clientList;
//                        clientNameHash[ip] = name;
                    }
                }
            }
        }
    }
}

