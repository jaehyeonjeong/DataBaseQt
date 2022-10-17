#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <QWidget>

class Client;                       /*client 정보를 가져오기 위해 Client class*/
class QMenu;                        /*QMenu를 가져오기 위한 QMenu class*/
class QTreeWidgetItem;              /*QTreeWidget을 가져오기 위한 QTreeWidgetItem class*/


namespace Ui {
class ClientManager;                /*ClientManager ui를 할당받는 클래스*/
}

class ClientManager : public QWidget
{
    Q_OBJECT

public:
    explicit ClientManager(QWidget *parent = nullptr);
    ~ClientManager();
   // QString getClientName();

signals:
    void ClientAdded(QString);

private slots:
    void on_ClientTreeWidget_itemClicked(QTreeWidgetItem *item, int column);        /*clientTreeWidget의 커서를 불러오는 슬롯*/
    void showContextMenu(const QPoint &);                                           /*마우스의 좌표에 따른 호출 슬롯*/
    void removeItem();                                                              /*고객 정보 제거 슬롯*/
    void on_InputButton_clicked();                                                  /*고객 정보 추가 버튼 슬롯*/
    void on_CancelButton_clicked();                                                 /*고객 정보 추가 취소 버튼 슬롯*/
    void on_ModifyButton_clicked();                                                 /*고객 정보 수정 버튼 슬롯*/
    void on_SearchButton_clicked();                                                 /*고객 정보 검색 버튼 슬롯*/

    void on_ClientSearchTree_itemClicked(QTreeWidgetItem *item, int column);        /*ClientSearchTreeWidget의 커서를 불러오는 슬롯*/

private:
    Ui::ClientManager *ui;                              /*UI 인자(해당되는 위젯 호출)*/
    int makeID();                                       /*아이디 할당 변수*/
    QMap<int, Client*> clientList;                      /*맵 형태 고객 리스트 변수*/
    QMenu* menu;                                        /*메뉴 변수*/
};

#endif // CLIENTMANAGER_H
