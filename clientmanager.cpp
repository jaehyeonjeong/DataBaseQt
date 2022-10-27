#include "clientmanager.h"
#include "ui_clientmanager.h"
#include "client.h"

#include <QFile>
#include <QMenu>
#include <iostream>

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
}

ClientManager::~ClientManager()
{
    delete ui;

    /*파일 저장*/
    QFile file("clientlist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) /*쓰기 형태인 텍스트 텍스트형태로 파일 열기*/
        return;

    QTextStream out(&file);
    for (const auto& v : clientList) {
        Client* c = v;
        out << c->id() << ", " << c->getName() << ", ";
        out << c->getPhoneNumber() << ", ";
        out << c->getAddress() << "\n";         /*고객의 아이디, 성함, 전화번호, 주소 저장*/
    }
    file.close( );
    //close의 반환형은 int 인데 정상적으로 파일을 닫았을 땐 0을 반환하고 그렇지 않을땐 EOF (-1)을 반환합니다.
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
}


void ClientManager::on_CancelButton_clicked()
{
    ui->CIDLineEdit->setText("");
    ui->CNameLineEdit->setText("");
    ui->CPhoneLineEdit->setText("");
    ui->CEmailLineEdit->setText("");
}

void ClientManager::on_ModifyButton_clicked()
{
    QTreeWidgetItem* item = ui->ClientTreeWidget->currentItem();
    if(item != nullptr) {
        int index = ui->ClientTreeWidget->currentIndex().row();
        int key = item->text(0).toInt();
        Client* c = clientList[key];
        QString name, number, address;
        name = ui->CNameLineEdit->text();
        number = ui->CPhoneLineEdit->text();
        address = ui->CEmailLineEdit->text();
        c->setName(name);
        c->setPhoneNumber(number);
        c->setAddress(address);
        clientList[key] = c;
        emit TCPClientModify(key, name, index);
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
    ui->ClientSearchTree->clear();
    int i = ui->SearchComboBox->currentIndex();
    auto flag = (i)? Qt::MatchCaseSensitive|Qt::MatchContains
                   : Qt::MatchCaseSensitive;
    {
        auto items = ui->ClientTreeWidget->findItems
                (ui->SearchLineEdit->text(), flag, i);

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


void ClientManager::on_ClientSearchTree_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->CNameLineEdit->setText(item->text(1));
}

