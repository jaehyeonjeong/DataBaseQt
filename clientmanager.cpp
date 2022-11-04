#include "clientmanager.h"
#include "ui_clientmanager.h"
#include "client.h"

#include <QFile>
#include <QMenu>
#include <iostream>

/*Oracle SQL 연동을 위한 헤더*/
#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


using namespace std;

ClientManager::ClientManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientManager)
{
    ui->setupUi(this);
    ui->toolBox->setCurrentIndex(0);                                    /*toolbox의 시작 위치는 0번째 부터(Input인자)*/

    QAction* removeAction = new QAction(tr("&Remove"));                 /*제거 액션 할당*/
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));     /*removeAction 연결*/

    menu = new QMenu;                                                   /*메뉴 할당*/
    menu->addAction(removeAction);                                      /*메뉴에 removeAction 추가*/
    ui->ClientTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);  /*contextMenuEvent() 처리기가 호출됨을 의미*/
    connect(ui->ClientTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    /*데이터 보내기*/
    connect(ui->CNameLineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(ClientAdded(QString)));

    ui->CPhoneLineEdit->setCursorPosition(1);
    //connect(this, SIGNAL(TCPClientAdded(int,QString)), this, SLOT(loadData()));
}

void ClientManager::loadData()                  /*고객의 정보를 택스트로 저장*/
{
    /*파일 불러오기*/
    QFile file("clientlist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");
        if(row.size()) {
            int id = row[0].toInt();
            Client* c = new Client(id, row[1], row[2], row[3]); /*고객의 데이터를 1행의 각열로 구분 (1행 4열)*/
            ui->ClientTreeWidget->addTopLevelItem(c);           /*고객의 정보를 리스트에 저장*/
            clientList.insert(id, c);                           /*map을 써서 insert로 데이터를 삽입*/
            emit TCPClientAdded(id, row[1]);                    /*채팅 서버에 해당하는 고객을 추가*/
            emit ClientAdded(row[1]);
        }
    }
    file.close( );

//    QModelIndex i = ui->ClientTreeWidget->currentIndex();
//    ui->ClientTreeWidget->setRowHidden(0, i, true);
//    ui->ClientTreeWidget->setRowHidden(1, i, true);
}

ClientManager::~ClientManager()
{
    delete ui;

    /*파일 저장*/
    QFile file("clientlist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) /*쓰기 형태인 텍스트 텍스트형태로 파일 열기*/
        return;

    QTextStream out(&file);
    //out << "clientID, clientName, clientPhone, clientEmail\n";
    for (const auto& v : clientList) {
        Client* c = v;
        out << c->id() << ", " << c->getName() << ", ";
        out << c->getPhoneNumber() << ", ";
        out << c->getAddress() << "\n";         /*고객의 아이디, 성함, 전화번호, 주소 저장*/
    }
    file.close( );
    //close의 반환형은 int 인데 정상적으로 파일을 닫았을 땐 0을 반환하고 그렇지 않을땐 EOF (-1)을 반환합니다.
}

/*database에 연동이 되는지 확인하는 함수*/
bool ClientManager::clientDataConnection()
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

int ClientManager::makeID( )        /*고객의 아이디 자동 할당*/
{
    if(clientList.size( ) == 0) {       /*고객의 정보에 어떤 데이터도 없을 때*/
        return 1;                   /*ID를 1부터 시작*/
    } else {
        auto id = clientList.lastKey();   /*고객정보중 최근 정보의 id에 접근*/
        return ++id;                    /*최근 아이디에 + 1 연산*/
    }
}

void ClientManager::removeItem()                /*아이템을 제거하는 함수*/
{
    QTreeWidgetItem* item = ui->ClientTreeWidget->currentItem();        /*아이템 할당*/
    if(item != nullptr) {                                   /*데이터가 없지 않는다면*/
        int index = ui->ClientTreeWidget->currentIndex().row();     /*각열에 맞는 인덱스 반환*/
        clientList.remove(item->text(0).toInt());           /*해당 아이디를 리스트에서 제거*/
        ui->ClientTreeWidget->takeTopLevelItem              /*고객의 정보 리스트에서 아이템을 지움*/
                (ui->ClientTreeWidget->indexOfTopLevelItem(item));
        ui->ClientTreeWidget->update();                 /*최신화*/
        emit ClientRemove(index);                   /*인덱스 값을 시그널로 보냄*/

    }
    if (!clientDataConnection( )) return;           /*고객 데이터베이스에 접근하지 못한 경우*/
    QSqlQueryModel queryModel;
    queryModel.setQuery(QString("CALL DELETE_CUST(%1)").arg(item->text(0).toInt()));     /*데이터 베이스에서 삭제할 고객 정보*/
}

void ClientManager::showContextMenu(const QPoint &pos)      /*컨택스트 메뉴*/
{
    QPoint globalPos = ui->ClientTreeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}


void ClientManager::on_InputButton_clicked()        /*input버튼 클릭 시 발생하는 이벤트 핸들러*/
{
    QString name, number, address;
    int id = makeID( );
    /**/
    name = ui->CNameLineEdit->text();
    number = ui->CPhoneLineEdit->text();
    address = ui->CEmailLineEdit->text();
    if(name.length()) {
        Client* c = new Client(id, name, number, address);
        clientList.insert(id, c);
        ui->ClientTreeWidget->addTopLevelItem(c);
        emit TCPClientAdded(id, name);
        emit ClientAdded(name);
    }

    /*고객 데이터베이스가 연결되지 않았을 경우*/
    if (!clientDataConnection( )) return;

    QSqlQueryModel queryModel;
    queryModel.setQuery(QString("CALL INSERT_CUST(%1, '%2', '%3', '%4')").arg(id).arg(name).arg(number).arg(address));     /*고객 정보로 들어갈 데이터*/
}


void ClientManager::on_CancelButton_clicked()       /*고객 관리 정보 에디터의 모든 라인 에디터를 비워주는 버튼 슬롯*/
{
    ui->CIDLineEdit->setText("");
    ui->CNameLineEdit->setText("");
    ui->CPhoneLineEdit->setText("");
    ui->CEmailLineEdit->setText("");
}

void ClientManager::on_ModifyButton_clicked()           /*고객 관리 데이터를 수정하기 위한 버튼 슬롯*/
{
    QTreeWidgetItem* item = ui->ClientTreeWidget->currentItem();    /*고객의 트리 리스트의 아이템을 할당*/
    if(item != nullptr) {
        int index = ui->ClientTreeWidget->currentIndex().row();     /*인덱스 값은 할당된 아이템의 1행 부분*/
        int key = item->text(0).toInt();                /*id가 고객정보의 키가 되어버림 */
        Client* c = clientList[key];                    /*고객 관리 데이터의 key값 할당*/
        QString name, number, address;
        name = ui->CNameLineEdit->text();               /*QString 타입으로 지정된 변수의 라인에디터 변수 재선언*/
        number = ui->CPhoneLineEdit->text();
        address = ui->CEmailLineEdit->text();
        c->setName(name);                               /*이름, 전화번호, 이메일 수정*/
        c->setPhoneNumber(number);
        c->setAddress(address);
        clientList[key] = c;
        emit TCPClientModify(key, name, index);         /*고객의 데이터를 수정할 시 서버의 클라이언트 리스트 에도 수정*/

        if (!clientDataConnection( )) return;           /*고객 데이터베이스에 접근하지 못한 경우*/
        QSqlQueryModel queryModel;
        queryModel.setQuery(QString("CALL UPDATE_CUST(%1, '%2', '%3', '%4')")
                            .arg(key).arg(name).arg(number).arg(address));     /*데이터 베이스에서 수정할 고객 정보*/
    }
}


void ClientManager::on_ClientTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->CIDLineEdit->setText(item->text(0));
    ui->CNameLineEdit->setText(item->text(1));
    ui->CPhoneLineEdit->setText(item->text(2));
    ui->CEmailLineEdit->setText(item->text(3));

    ui->toolBox->setCurrentIndex(0);
    //emit TCPClientAdded(item->text(0).toInt(), item->text(1));
}


void ClientManager::on_SearchButton_clicked()
{
    ui->ClientSearchTree->clear();                      /*검색 리스트를 클리어*/
    int i = ui->SearchComboBox->currentIndex();         /*검색 리스트의 인덱스를 할당*/
    auto flag = (i)? Qt::MatchCaseSensitive|Qt::MatchContains      /*매칭의 조건 옵션 추가*/
                   : Qt::MatchCaseSensitive;            /*Qt::MatchCaseSensitive : 대소문자 구분*/
    {                                                   /*Qt::MatchContains : 검색어가 항목에 포함되어 있는지를 구분*/
        auto items = ui->ClientTreeWidget->findItems
                (ui->SearchLineEdit->text(), flag, i);  /*검색 옵션에 맞는 정보를 컬럼별로 찾음*/

        foreach(auto i, items) {
            Client* c = static_cast<Client*>(i);
            int id = c->id();
            QString name = c->getName();
            QString number = c->getPhoneNumber();
            QString address = c->getAddress();
            Client* item = new Client(id, name, number, address);
            ui->ClientSearchTree->addTopLevelItem(item);
        }
    }
}

/*검색 리스트의 해당 아이템을 클릭 하면 고객의 성함을 에디터에 연결*/
void ClientManager::on_ClientSearchTree_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->CNameLineEdit->setText(item->text(1));
}

