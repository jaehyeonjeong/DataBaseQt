#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clientmanager.h"
#include "productmanager.h"
#include "shoppingmanager.h"
#include "chetting.h"
#include "tcpclient.h"
#include "tcpserver.h"

#include <QMdiSubWindow>
#include <QApplication>
#include <QMessageBox>
#include <QLabel>
#include <QFileDialog>

/*Oracle SQL 연동을 위한 헤더*/
#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

static bool createConnection( )     /*데이터 베이스 연결 확인*/
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");   /*추가하려는 데이터베이스 종류는 QODBC(Qt Oracle DataBase)*/
    db.setDatabaseName("Oracle11g");            /*데이터베이스 이름*/
    db.setUserName("projectDB");                /*데이터 베이스 계정 명*/
    db.setPassword("1234");                     /*데이터 베이스 비밀번호*/
    if (!db.open()) {
        qDebug() << db.lastError().text();
    } else {
        qDebug("success");
    }

    return true;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Ostem Project");          /*프로젝트 타이틀*/
    clientmanager = new ClientManager(this);        /*고객관리 클래스 변수 초기화*/
    productmanager = new ProductManager(this);      /*상품관리 클래스 변수 초기화*/
    shoppingmanager = new ShoppingManager(this);    /*구매정보관리 클래스 변수 초기화*/

    tcpserver = new TCPServer(this);            /*서버 및 로그 위젯 클래스 변수 초기화*/
    tcpclient = new TCPClient(this);            /*클라이언트 채팅방 클래스 변수 초기화*/

    chettingapp = new Chetting;     /*관리자 채팅방 초기화*/

    ui->tabWidget->addTab(clientmanager, tr("&ClientTab"));     /*고객관리 Ui를 탭으로 추가*/
    ui->tabWidget->addTab(productmanager, tr("&ProductTab"));   /*상품관리 Ui를 탭으로 추가*/

    /*쇼핑리스트를 보여주는 토드*/
    subWindow = new QMdiSubWindow;              /*서브윈도우 클래스 변수 초기화*/
    subWindow->setWidget(shoppingmanager);      /*구매정보관리 Ui 위젯으로 세팅*/
    subWindow->setAttribute(Qt::WA_DeleteOnClose);  /*창 속성을 Qt::WA_DeleteOnClose로 설정하여 Qt가 창을 닫는 즉시 메모리에서 창 삭제를 처리하도록 합니다.*/
    subWindow->setWindowTitle("Shopping Window");
    subWindow->setGeometry(0, 0, 800, 390);      /*매개인자 1) x좌표, 2) y좌표, 3) 위젯의 width, 4) 위젯의 height*/
    subWindow->setWindowFlags(/*Qt::WindowTitleHint|Qt::WindowMinimizeButtonHint*/
                              Qt::FramelessWindowHint); /*MDI에서 생성되는 윈도우의 축소/확대/닫기 버튼을 보이지 않게 함.*/
    ui->mdiArea->addSubWindow(subWindow);        /*MDI에 서브 윈도우 추가*/

    /*TCP 서버와 로그 상태를 담당하는 윈도우*/
    TcpSubWindow[0] = new QMdiSubWindow;
    TcpSubWindow[0]->setWidget(tcpserver);
    TcpSubWindow[0]->setAttribute(Qt::WA_DeleteOnClose);
    TcpSubWindow[0]->setWindowTitle("TcpServer");
    TcpSubWindow[0]->setWindowFlags(/*Qt::WindowTitleHint|Qt::WindowMinimizeButtonHint*/
                                    Qt::FramelessWindowHint);
    ui->mdiArea->addSubWindow(TcpSubWindow[0]);
    TcpSubWindow[0]->setGeometry(0, 390, 800, 390);/*매개인자 1) x좌표, 2) y좌표, 3) 위젯의 width, 4) 위젯의 height*/

    /*고객 윈도우 그러나 프로젝트를 출력하면 보이지 않음 메인윈도우를 늘려야지 볼 수 있음*/
    TcpSubWindow[1] = new QMdiSubWindow;
    TcpSubWindow[1]->setWidget(tcpclient);
    TcpSubWindow[1]->setAttribute(Qt::WA_DeleteOnClose);
    TcpSubWindow[1]->setWindowTitle("client chetting");
    TcpSubWindow[1]->setWindowFlags(Qt::WindowTitleHint|Qt::WindowMinimizeButtonHint
                                    /*Qt::FramelessWindowHint*/);/*최소의 버튼만 표시*/
    ui->mdiArea->addSubWindow(TcpSubWindow[1]);
    TcpSubWindow[1]->setGeometry(800, 0, 340, 380);/*매개인자 1) x좌표, 2) y좌표, 3) 위젯의 width, 4) 위젯의 height*/

    /*탭과 MDIAREA 스핀로드*/
    QList<int> list;
    list << 700 << 800;      /*메인윈도우의 전체 넓이 1500중 700은 탭 위젯, 800은 MDI위젯을 출력*/
    ui->splitter->setSizes(list);   /*스플리터 사이즈를 세팅*/

    /*메인윈도우에서 데이터 커넥트*/
    connect(clientmanager, SIGNAL(ClientAdded(QString)),    /*고객의 성함을 넘기는 신호를 보냄*/
            shoppingmanager, SLOT(CreceiveData(QString)));  /*고객의 성함 신호를 받아 구매정보 위젯에 고객의 성함을 갱신*/

    connect(productmanager, SIGNAL(ProductAdded(QString)),  /*상품의 이름을 넘기는 신호를 보냄*/
            shoppingmanager, SLOT(PreceiveData(QString)));  /*상품의 이름 신호를 받아 구매정보 위젯에 상품의 이름을 갱신*/

    connect(productmanager, SIGNAL(ProductPrices(QString)), /*상품의 가격을 넘기는 신호를 보냄*/
            shoppingmanager, SLOT(PreceivePrice(QString))); /*상품의 가격 신호를 받아 구매정보 위젯에 상품의 가격을 갱신*/

    this->resize(1500, 840);        /*출력되는 프로젝트의 넓이와 높이를 조젛 width : 1500, height : 840*/
    this->setMaximumWidth(1500);    /*Mdi의 남는 영역이 보이지 않도록 최대 넓이 지정*/
    this->setMaximumHeight(840);    /*Mdi의 남는 영역이 보이지 않도록 최대 높이 지정*/

    connect(clientmanager, SIGNAL(TCPClientAdded(int, QString)), /*고객관리 위젯 보내는 */
            tcpserver, SLOT(addClient(int, QString)));
    clientmanager->loadData();

    connect(clientmanager, SIGNAL(ClientRemove(int)),   /**/
            tcpserver, SLOT(removeClient(int)));

    connect(clientmanager, SIGNAL(TCPClientModify(int, QString,int)),
            tcpserver, SLOT(modifyClient(int, QString,int)));

    /*관리자 클라이언트에서 연결하는 요소는 반드시 생성자에 작성*/
    connect(chettingapp, SIGNAL(TCPSignal(int, QString)),
            tcpserver, SLOT(receiveManager(int, QString)));

    /*메인 윈도우에 있는 메뉴바에 테이블을 불러오면 메인윈도우 창닫기 버튼 비활성화*/
    tableview = new QTableView;
    if(tableview->isActiveWindow() != false)
    {
        /*여기서 어떻게 하면 테이블을 불러올 때마다 창닫기를 비활성화 시킬 수 있을까?*/
        this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


/*클라이언트용 채팅활성화, chetting 툴바 버튼 클릭 시 프로젝트 맨앞에서 출력*/
void MainWindow::on_actionchatting_triggered()
{
    qDebug(".......");
    tcpclient = new TCPClient;
    tcpclient->setWindowFlag(Qt::WindowStaysOnTopHint);
    tcpclient->setAttribute(Qt::WA_DeleteOnClose);
    //윈도우는 항상 위에 위치
    tcpclient->move(100, 600);  //x = 100, y = 600에 위치
    tcpclient->show();

    /*서버로 부터 보내는 시그널 값을 클라이언트로 커넥트*/
    connect(tcpclient, SIGNAL(compareName(QString)),
            tcpserver, SLOT(CheckLogIn(QString)));

    connect(tcpserver, SIGNAL(SendLogInChecked(int)),
            tcpclient, SLOT(receiveSignal(int)));
}

/*관리자용 채팅 활성화 manager 툴바 버튼 클릭 시 프로젝트 맨앞에서 출력*/
void MainWindow::on_actionmanager_triggered()
{
    chettingapp->setWindowFlag(Qt::WindowStaysOnTopHint);
    chettingapp->setWindowTitle("manager");
    //윈도우는 항상 위에 위치
    chettingapp->move(300, 600);  //x = 300, y = 600에 위치
    chettingapp->show();
}


int MainWindow::on_actionshoppingDB_triggered()
{
    if (!createConnection( )) return 1;

    queryModel = new QSqlQueryModel;
    queryModel->setQuery("select o_id, c_name, g_name, o_date, o_quan, o_allprice from ORDERS order by o_id asc");    /*상품 데이터베이스 출력 쿼리문*/
    queryModel->setHeaderData(0, Qt::Horizontal, QObject::tr("o_id"));
    queryModel->setHeaderData(1, Qt::Horizontal, QObject::tr("c_name"));
    queryModel->setHeaderData(2, Qt::Horizontal, QObject::tr("g_name"));
    queryModel->setHeaderData(3, Qt::Horizontal, QObject::tr("o_date"));
    queryModel->setHeaderData(4, Qt::Horizontal, QObject::tr("o_quan"));
    queryModel->setHeaderData(5, Qt::Horizontal, QObject::tr("o_allprice"));


    //tableview = new QTableView;
    tableview->setModel(queryModel);
    tableview->setWindowTitle(QObject::tr("Shopping DataBase"));
    tableview->show( );
    flag = 3;
    qDebug() << flag;
    if(!isActiveWindow())
    {
        flag = 1000;
        qDebug() << flag;
    }
}

