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

    /*쇼핑리스트를 보여주는 토드*/
    subWindow = new QMdiSubWindow;
    subWindow->setWidget(shoppingmanager);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setWindowTitle("Shopping Window");
    subWindow->setGeometry(0, 0, 800, 400);
    subWindow->setWindowFlags(/*Qt::WindowTitleHint|Qt::WindowMinimizeButtonHint*/
                              Qt::FramelessWindowHint);
    ui->mdiArea->addSubWindow(subWindow);

    /*TCP 서버와 로그 상태를 담당하는 윈도우*/
    TcpSubWindow[0] = new QMdiSubWindow;
    TcpSubWindow[0]->setWidget(tcpserver);
    TcpSubWindow[0]->setAttribute(Qt::WA_DeleteOnClose);
    TcpSubWindow[0]->setWindowTitle("TcpServer");
    TcpSubWindow[0]->setWindowFlags(/*Qt::WindowTitleHint|Qt::WindowMinimizeButtonHint*/
                              Qt::FramelessWindowHint);
    ui->mdiArea->addSubWindow(TcpSubWindow[0]);
    TcpSubWindow[0]->setGeometry(0, 400, 450, 360);

    /*고객 윈도우 그러나 프로젝트를 출력하면 보이지 않음*/
    TcpSubWindow[1] = new QMdiSubWindow;
    TcpSubWindow[1]->setWidget(tcpclient);
    TcpSubWindow[1]->setAttribute(Qt::WA_DeleteOnClose);
    TcpSubWindow[1]->setWindowTitle("client chetting");
    TcpSubWindow[1]->setWindowFlags(Qt::WindowTitleHint|Qt::WindowMinimizeButtonHint
                              /*Qt::FramelessWindowHint*/);
    ui->mdiArea->addSubWindow(TcpSubWindow[1]);
    TcpSubWindow[1]->setGeometry(800, 0, 340, 380);

    /*관리자 채팅을 보여주는 코드*/
    TcpSubWindow[2] = new QMdiSubWindow;
    TcpSubWindow[2]->setWidget(chettingapp);
    TcpSubWindow[2]->setAttribute(Qt::WA_DeleteOnClose);
    TcpSubWindow[2]->setWindowTitle("manager Chetting Application");
    TcpSubWindow[2]->setWindowFlags(/*Qt::WindowTitleHint|Qt::WindowMinimizeButtonHint*/
                              Qt::FramelessWindowHint);
    ui->mdiArea->addSubWindow(TcpSubWindow[2]);
    TcpSubWindow[2]->setGeometry(450, 390, 340, 370);

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
    connect(clientmanager, SIGNAL(ClientAdded(QString)),
            shoppingmanager, SLOT(CreceiveData(QString)));

    connect(productmanager, SIGNAL(ProductAdded(QString)),
            shoppingmanager, SLOT(PreceiveData(QString)));

    connect(productmanager, SIGNAL(ProductPrices(QString)),
            shoppingmanager, SLOT(PreceivePrice(QString)));

    this->resize(1500, 840);

    //    connect(chettingapp->clientsocket,
    //            &QAbstractSocket::errorOccurred,
    //            [=]{qDebug() << chettingapp->clientsocket->errorString();
    //    });
    //connect(chettingapp, SIGNAL(readyRead()), SLOT(sendData()));

    connect(clientmanager, SIGNAL(TCPClientAdded(int, QString)),
            tcpserver, SLOT(addClient(int, QString)));
    clientmanager->loadData();

    connect(chettingapp, SIGNAL(TCPSignal(int, QString)),
            tcpserver, SLOT(receiveManager(int, QString)));

    connect(clientmanager, SIGNAL(ClientRemove(int)),
            tcpserver, SLOT(removeClient(int)));

    connect(clientmanager, SIGNAL(TCPClientModify(QString,int)),
            tcpserver, SLOT(modifyClient(QString,int)));

}

MainWindow::~MainWindow()
{
    delete ui;
}


/*클라이언트용 채팅활성화 chetting 툴바 버튼 클릭시 프로젝트 맨앞에서 출력*/
void MainWindow::on_actionchatting_triggered()
{
    qDebug(".......");
    tcpclient = new TCPClient;
    tcpclient->setWindowFlag(Qt::WindowStaysOnTopHint);
    tcpclient->show();
}

