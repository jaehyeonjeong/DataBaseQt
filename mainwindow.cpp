#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clientmanager.h"
#include "productmanager.h"
#include "shoppingmanager.h"
#include "chetting.h"
#include "tcpclient.h"
#include "tcpserver.h"

#include <QMdiSubWindow>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    clientmanager = new ClientManager(this);
    productmanager = new ProductManager(this);
    shoppingmanager = new ShoppingManager(this);

    //    TCPServer* tcpserver;
    //    TCPClient* tcpclient;
    tcpserver = new TCPServer(this);
    tcpclient = new TCPClient(this);
    //logwindow = new tcplog(this);

    chettingapp = new Chetting(this);


    ui->tabWidget->addTab(clientmanager, tr("&ClientTab"));
    ui->tabWidget->addTab(productmanager, tr("&ProductTab"));
    //ui->tabWidget->addTab(shoppingmanager, tr("&ShoppingTab"));

    //    QMdiArea mdiArea;
    //    QMdiSubWindow *subWindow1 = new QMdiSubWindow;
    //    subWindow1->setWidget(internalWidget1);
    //    subWindow1->setAttribute(Qt::WA_DeleteOnClose);
    //    mdiArea.addSubWindow(subWindow1);

    //    QMdiSubWindow *subWindow2 =
    //        mdiArea.addSubWindow(internalWidget2);

    subWindow = new QMdiSubWindow;
    subWindow->setWidget(shoppingmanager);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setWindowTitle("Shopping Window");
    subWindow->setGeometry(0, 0, 450, 400);
    ui->mdiArea->addSubWindow(subWindow);

    /*TCP 채팅 위젯 추가*/
    TcpSubWindow[0] = new QMdiSubWindow;
    TcpSubWindow[0]->setWidget(tcpserver);
    TcpSubWindow[0]->setAttribute(Qt::WA_DeleteOnClose);
    TcpSubWindow[0]->setWindowTitle("TcpServer");
    ui->mdiArea->addSubWindow(TcpSubWindow[0]);
    TcpSubWindow[0]->setGeometry(450,0, 340, 400);

    TcpSubWindow[1] = new QMdiSubWindow;
    TcpSubWindow[1]->setWidget(tcpclient);
    TcpSubWindow[1]->setAttribute(Qt::WA_DeleteOnClose);
    TcpSubWindow[1]->setWindowTitle("client chetting");
    ui->mdiArea->addSubWindow(TcpSubWindow[1]);
    TcpSubWindow[1]->setGeometry(0, 400, 300, 360);

    TcpSubWindow[2] = new QMdiSubWindow;
    TcpSubWindow[2]->setWidget(chettingapp);
    TcpSubWindow[2]->setAttribute(Qt::WA_DeleteOnClose);
    TcpSubWindow[2]->setWindowTitle("manager Chetting Application");
    ui->mdiArea->addSubWindow(TcpSubWindow[2]);
    TcpSubWindow[2]->setGeometry(300, 400, 490, 360);

//    TcpSubWindow[3] = new QMdiSubWindow;
//    TcpSubWindow[3]->setWidget(logwindow);
//    TcpSubWindow[3]->setAttribute(Qt::WA_DeleteOnClose);
//    TcpSubWindow[3]->setWindowTitle("chetting log Window");
//    ui->mdiArea->addSubWindow(TcpSubWindow[3]);
//    TcpSubWindow[3]->setGeometry(450, 100, 340, 300);


    /*탭과 MDIAREA 스핀로드*/
    QList<int> list;
    list << 700 << 800;
    ui->splitter->setSizes(list);

    /*메인윈도우에서 데이터 커넥트*/
    connect(clientmanager, SIGNAL(ClientAdded(/*int,*/ QString)),
            shoppingmanager, SLOT(CreceiveData(/*int,*/ QString)));

    connect(productmanager, SIGNAL(ProductAdded(QString)),
            shoppingmanager, SLOT(PreceiveData(QString)));

    connect(productmanager, SIGNAL(ProductPrices(QString)),
            shoppingmanager, SLOT(PreceivePrice(QString)));

    this->resize(1500, 800);

    //    connect(chettingapp->clientsocket,
    //            &QAbstractSocket::errorOccurred,
    //            [=]{qDebug() << chettingapp->clientsocket->errorString();
    //    });
    //connect(chettingapp, SIGNAL(readyRead()), SLOT(sendData()));

    connect(clientmanager, SIGNAL(ClientAdded(QString)),
            tcpclient, SLOT(CReceiveData(QString)));

    connect(tcpclient, SIGNAL(ButtonSignal(QString)),
            chettingapp, SLOT(receiveClient(QString)));

    connect(tcpclient, SIGNAL(ButtonSignal(QString)),
            chettingapp, SLOT(receiveTCPClientName(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete clientmanager;
    delete productmanager;
    delete shoppingmanager;

//    delete logwindow;

//    for(int i = 0; i < 4; i++)
//    {
//        delete TcpSubWindow[i];
//    }
    delete *TcpSubWindow;
}

