
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ClientManager;    //1
class ProductManager;   //2
class ShoppingManager;  //3
class TCPServer;        //4
class TCPClient;        //5
class Chetting;         //6
class QTcpSocket;       //7
class tcplog;           //8  /*mainwindow.cpp에 들어갈 클래스 8개 추가*/

class QMdiSubWindow;            /*mainwinodw.cpp 파일에 듫어갈 서브 윈도우*/
class QSqlQueryModel;       /*sql query문과 접근하기 위한 모델 클래스*/
class QTableView;           /*데이터 정보를 보여주는 테이블 클래스*/

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }      //ui지원
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_actionchatting_triggered();     //ui에서 생성된 채팅 클라이언트를 불러오는 툴바 버튼 이벤트 핸들러

    void on_actionmanager_triggered();      //ui에서 생성된 매니저 클라이언트를 불러오는 툴바 버튼 이벤트 핸들러

    int on_actionshoppingDB_triggered();

private:
    Ui::MainWindow *ui;         //ui변수
    ClientManager* clientmanager;   //고객관리 클래스 변수
    ProductManager* productmanager;     //상품관리 클래스 변수
    ShoppingManager* shoppingmanager;   //구매 정보 관리 클래스 변수
    Chetting* chettingapp;  //관리자 시각의 클라이언트 클래스 변수

    TCPServer* tcpserver;   //채팅 접속자와 로그를 기록할 수 있는 서버 클래스 변수
    TCPClient* tcpclient;   //고객 시각의 채팅 클라이언트 클래스 변수

    QMdiSubWindow* subWindow;   //MDI에서 구매정보를 불러오는 서브 윈도우 클래스 변수
    QMdiSubWindow* TcpSubWindow[2]; //MDI에서 2개 더 추가할 서브윈도우 클래스 변수

    QSqlQueryModel* queryModel; /*쿼리문 모델 변수*/
    QTableView *tableview;      /*테이블을 보여주는 클래스*/
    int flag = 1000;
};
#endif // MAINWINDOW_H
