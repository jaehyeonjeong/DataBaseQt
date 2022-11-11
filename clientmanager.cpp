#include "clientmanager.h"
#include "ui_clientmanager.h"

#include <QFile>
#include <QMenu>
#include <iostream>
#include <QTableWidgetItem>

/*Oracle SQL 연동을 위한 헤더*/
#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>


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

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);         /*테이블 뷰에서 메뉴 정의*/
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenuTable(QPoint)));                  /*컨텍스트 메뉴가 나올 수 있게끔 생성*/

    /*고객의 성함 데이터 보내기*/
    connect(ui->CNameLineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(ClientAdded(QString)));

    ui->CPhoneLineEdit->setCursorPosition(1);       /*전화번호 자동라인 맞춤*/

    //connect(this, SIGNAL(TCPClientAdded(int,QString)), this, SLOT(loadData()));

    SearchModel = new QStandardItemModel(0, 4);                         /*행렬중 row = 0, column = 4로 초기화*/
    SearchModel->setHeaderData(0, Qt::Horizontal, tr("ID"));            /*1번째 column 이름을 ID*/
    SearchModel->setHeaderData(1, Qt::Horizontal, tr("Name"));          /*2번째 column 이름을 Name*/
    SearchModel->setHeaderData(2, Qt::Horizontal, tr("Phone Number"));  /*3번째 column 이름을 Phone Number*/
    SearchModel->setHeaderData(3, Qt::Horizontal, tr("Email"));         /*4번째 column 이름을 Email로 설정*/
    ui->searchTableView->setModel(SearchModel);                         /*테이블 뷰 위젯에 SearchModel 추가*/
}

void ClientManager::loadData()                  /*고객의 정보를 택스트로 저장*/
{
    /*데이터 베이스의 데이터들을 불러오기*/
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "clientConnection");
    /*추가하려는 데이터베이스 종류는 QODBC(Qt Oracle DataBase)*/
    db.setDatabaseName("Oracle11g");            /*데이터베이스 이름*/
    db.setUserName("projectDB");                /*데이터 베이스 계정 명*/
    db.setPassword("1234");                     /*데이터 베이스 비밀번호*/

    if (db.open()) {
        clientquery = new QSqlQuery(db);
        ClientModel = new QSqlTableModel(this, db);
        //SearchModel = new QSqlTableModel(this, db);     /*검색용 테이븧 모델 추가*/

        ClientModel->setTable("CUST");

        clientquery->exec("SELECT * FROM CUST ORDER BY C_ID ASC");
        ClientModel->select();

        //SearchModel->setTable("SEARCH_CUST");

        /*고객 데이터베이스 출력 쿼리문*/
        ClientModel->setHeaderData(0, Qt::Horizontal, QObject::tr("c_id"));
        ClientModel->setHeaderData(1, Qt::Horizontal, QObject::tr("c_name"));
        ClientModel->setHeaderData(2, Qt::Horizontal, QObject::tr("c_phone"));
        ClientModel->setHeaderData(3, Qt::Horizontal, QObject::tr("c_email"));


        ui->tableView->setModel(ClientModel);
        ui->tableView->resizeColumnsToContents();
    }
    for(int i = 0; i < ClientModel->rowCount(); i++){                       /*로우 카운트를 이용하여 데이터를 가지고 오는 함수*/
        int id = ClientModel->data(ClientModel->index(i, 0)).toInt();       /*로우와 컬럼 값으로 아이디와 이름 할당*/
        QString name = ClientModel->data(ClientModel->index(i, 1)).toString();
        emit TCPClientAdded(id, name);
    }
}

ClientManager::~ClientManager()
{
    delete ui;

    QSqlDatabase db = QSqlDatabase::database("clientConnection");
    if(db.isOpen())                     /*데이터 베이스가 열려있다면*/
    {
        ClientModel->submitAll();
        /*보류 중인 모든 변경 사항을 제출하고 성공하면 true를 반환합니다.
         * 오류 시 false를 반환하고 lastError()를 사용하여 자세한 오류 정보를 얻을 수 있습니다.*/

        db.close();
        //db.removeDatabase("QODBC");     /*할당되어 있는 QODBC를 제거*/
        QSqlDatabase::removeDatabase("clientConnection");   /*해당된 이름의 데이터베이스를 제거*/
    }
}

int ClientManager::makeID( )        /*고객의 아이디 자동 할당*/
{
    if(ClientModel->rowCount() == 0) {       /*고객 데이터 베이스 정보에 어떤 데이터도 없을 때*/
        return 1;                   /*ID를 1부터 시작*/
    } else {
        int id = ui->CIDLineEdit->text().toInt();   /*원인은 모르겠지만 ui 라인 에디트로 내 코드는 접근해야 g함*/
        //int id = ClientModel->data(ClientModel->index(ClientModel->rowCount() - 1, 0)).toInt();
        /*고객정보중 최근 정보의 id에 접근*/
        qDebug() << "id : " << id;
        return ++id;                    /*최근 아이디에 + 1 연산*/
    }
}

void ClientManager::removeItem()                /*아이템을 제거하는 함수*/
{
    int idx = ui->CIDLineEdit->text().toInt();      /*채팅 서버 클라이언트에 보낼 인덱스*/
    qDebug() << "remove id : " << idx;

    if(idx){        /*index 값이 존재하는 경우*/
        clientquery->exec(QString("CALL DELETE_CUST(%1)").arg(idx));    /*해당하는 인덱스 데이터를 지우는 프로시져 호출*/
        ClientModel->select();      /*해당 테이블 호출*/
        //ui->tableView->setModel(ClientModel);   /*테이블 모델 셋*/
        ui->tableView->update();
        ui->tableView->resizeColumnsToContents();       /*데이터 사이즈에 맞게 열을 정렬*/
        emit ClientRemove(idx);     /*서버 대기방에 없는 이름의 클라이언트는 삭제하는 시그널*/
    }


}

void ClientManager::showContextMenuTable(const QPoint &pos)      /*컨택스트 메뉴*/
{
    QPoint globalPos = ui->tableView->mapToGlobal(pos);     /*마우스의 위치에 표시*/
    if(ui->tableView->indexAt(pos).isValid())               /*마우스로 테이블데이터의 우클릭을 했을 시*/
        menu->exec(globalPos);                              /*컨택스트 메뉴 실행*/
}


void ClientManager::on_InputButton_clicked()        /*input버튼 클릭 시 발생하는 이벤트 핸들러*/
{
    QString name, number, address;              /*고객 정보 테이블에 들어갈 데이터 변수*/
    int id = makeID( );                         /*아이디는 자동으로 생성*/
    //QModelIndex tIndex = ui->tableView->currentIndex();             /*테이블 뷰에 인덱스*/

    ui->CIDLineEdit->setText(QString::number(id));      /*아이디 라인에디트에 아이디 할당*/
    name = ui->CNameLineEdit->text();                   /*이름 라인에디트 이름 할당*/
    number = ui->CPhoneLineEdit->text();                /*전화번호 라인에디트 전화번호 할당*/
    address = ui->CEmailLineEdit->text();               /*이메일 라인에디트 이메일 할당*/
    if(id && name.length() && number.length() && address.length()) {
        //clientquery(ClientModel->database());       /*고객의 데이터베이스를 가져옴*/
        /*오라클 내의 프로시져를 사용*/
#if 1
        clientquery->exec(QString("CALL INSERT_CUST(%1, '%2', '%3', '%4')")
                          .arg(id).arg(name).arg(number).arg(address));
        ClientModel->select();                          /*릴레이션 테이블 호출*/

        ui->tableView->setModel(ClientModel);           /*테이븛 뷰에 띄우가*/
        ui->tableView->resizeColumnsToContents();       /*데이터 사이즈에 맞게 열을 정렬*/
#else

        /*데이터를 입력하기 전에 최근 데이터를 클릭하고 데이터를 입력 해야지 새롭게 데이터가 입력된다*/
        clientquery->prepare("INSERT INTO CUST VALUES (?, ?, ?, ?)");
        clientquery->bindValue(0, id);
        clientquery->bindValue(1, name);
        clientquery->bindValue(2, number);
        clientquery->bindValue(3, address);
        clientquery->exec();
        ClientModel->select();

        //ClientModel->QSqlQueryModel::setQuery("SELECT * FROM CUST ORDER BY C_ID");

        ui->tableView->setModel(ClientModel);           /*테이븛 뷰에 띄우가*/
#endif
        emit TCPClientAdded(id, name);      /*고객의 정보를 추가할 시 이름만 보내는 시그널 서버에서 데이터를 받고 갱신*/
    }
}


void ClientManager::on_CancelButton_clicked()       /*고객 관리 정보 에디터의 모든 라인 에디터를 비워주는 버튼 슬롯*/
{
    ui->CIDLineEdit->setText("");               /*고객의 아이디 에디트 공백*/
    ui->CNameLineEdit->setText("");             /*고객의 성함 에디트 공백*/
    ui->CPhoneLineEdit->setText("");            /*고객의 전화번호 에디트 공백*/
    ui->CEmailLineEdit->setText("");            /*고객의 이메일 에디트 공백*/
}

void ClientManager::on_ModifyButton_clicked()           /*고객 관리 데이터를 수정하기 위한 버튼 슬롯*/
{
    QModelIndex tIndex = ui->tableView->currentIndex(); /*테이블 뷰 인덱스 할당*/
    int index = ui->tableView->currentIndex().row();
    if(tIndex.isValid()){                   /*인덱스가 존재 한다면*/
        int ID = ui->CIDLineEdit->text().toInt();   /*정수형 데이터*/
        QString name = ui->CNameLineEdit->text();      /*QString 데이터*/
        QString phone = ui->CPhoneLineEdit->text();
        QString address = ui->CEmailLineEdit->text();
        //qDebug() << ID << name << phone << address;

        if(name.length() && phone.length() && address.length())
        {
            clientquery->exec(QString("CALL UPDATE_CUST(%1, '%2', '%3', '%4')")
                              .arg(ID).arg(name).arg(phone).arg(address));

            clientquery->exec(QString("select * from CUST ORDER BY C_ID"));
            ClientModel->select();

            ui->tableView->setModel(ClientModel);           /*수정 전*/
            emit TCPClientModify(ID, name, index);
            /*고객의 데이터를 수정할 시 서버의 클라이언트 리스트 에도 수정*/
        }
    }
}


void ClientManager::on_TBpushButton_clicked()
{
    SearchModel->clear();                                          /*SearhModel 초기화*/
    int i = ui->SearchComboBox->currentIndex();                    /*콤보박스에 해당하는 인덱스 변수 선언*/
    auto flag = (i) ? Qt::MatchCaseSensitive|Qt::MatchContains     /*검색 플래그 매칭 조건*/
                    : Qt::MatchCaseSensitive;
    QModelIndexList indexs = ClientModel->match(ClientModel->index(0, i),   /*match() 파라미터에 따른 검색 리스트 인덱스 나열*/
             Qt::EditRole, ui->SearchLineEdit->text(), -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexs){
        int id = ClientModel->data(ix.siblingAtColumn(0)).toInt(); //해당되는 열을 출력(id에 해당하는 모든 정보)
        QString name = ClientModel->data(ix.siblingAtColumn(1)).toString(); //name에 해당되는 열을 출력
        QString number = ClientModel->data(ix.siblingAtColumn(2)).toString(); //number에 해당되는 열을 출력
        QString email = ClientModel->data(ix.siblingAtColumn(3)).toString(); //address에 해당되는 열을 출력
        QStringList strings;
        strings << QString::number(id) << name << number << email;          //검색된 행에 아이디, 이름, 전화번호, 이메일을 strings에 순서대로 저장

        QList<QStandardItem *> items;                                       /*QStandardItme을 상속한 리스트 아이템 변수를 선언*/
        for(int i = 0; i < 4; i++){
            items.append(new QStandardItem(strings.at(i)));                 /*4번째 컬럼까지 데이터를 append*/
        }

        SearchModel->appendRow(items);                                      /*1개의 행 내용을 전부 출력*/
        SearchModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        SearchModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        SearchModel->setHeaderData(2, Qt::Horizontal, tr("Phone Number"));
        SearchModel->setHeaderData(3, Qt::Horizontal, tr("Email"));


        ui->searchTableView->resizeColumnsToContents();                     /*입력된 데이터의 크기 만큼 컬럼을 조정*/
    }
}

/*테이블 뷰 검색 리스트의 해당 아이템을 클릭하면 고객의 성함을 에디터에 연결*/
void ClientManager::on_searchTableView_clicked(const QModelIndex &index)
{
    QString name = index.sibling(index.row(), 1).data().toString();
    ui->CNameLineEdit->setText(name);
}


void ClientManager::on_removeButton_clicked()       /*해당하는 고객의 아이디중 최대값인 아이디를  구하는 코드*/
{
#if 1
    clientquery->prepare("SELECT MAX(C_ID) FROM CUST;");/*현존하는 아이디중 최댓값을 출력하는 쿼리문*/
    clientquery->exec();            /*쿼리문 실행*/
    while (clientquery->next()) {   /*쿼리문 진행*/
        qDebug() << clientquery->value(0).toInt();  /*쿼리값 디버깅*/
        ui->RemoveLineEdit->setText(QString::number(clientquery->value(0).toInt()));    /*아이디 라인 에디트체 자동 할당 실행*/
        ui->CIDLineEdit->setText(QString::number(clientquery->value(0).toInt()));       /*표시 라인 에디트에 자동 할당*/
        //ClientModel->select();
    }

#else
    /*클라이언트 데이터 정보중 ID가 가장 큰값인 아이디를 도출하기 위한 코드*/
    int leng = ClientModel->rowCount();
    //int pnum[10000] = {0,};
    int* pnum = (int *)malloc(sizeof(int) * leng);
    int tmp = 0;

    for(int i = 0; i < leng; i++)
    {
        pnum[i] = ClientModel->data(ClientModel->index(i, 0)).toInt();
    }

    for(int i = 0 ; i < leng - 1; i++)
    {
        for(int j = leng - 1; j > 0; j--)
        {
            if(pnum[i] < pnum[j])
            {
                tmp = pnum[i];
                pnum[i] = pnum[j];
                pnum[j] = tmp;
            }
        }
    }

    qDebug() << pnum[0] << " " << pnum[1] << " " << pnum[3];
    qDebug() << ClientModel->rowCount();
    ui->RemoveLineEdit->setText(QString::number(pnum[0]));
    ui->CIDLineEdit->setText(QString::number(pnum[0]));


#endif

    /*강사님께 질문 하기*/
    //ui->RemoveLineEdit->setText(ClientModel->query.record(0));

    //ui->tableView->(clientquery);           /*수정 전*/

}


void ClientManager::on_tableView_clicked(const QModelIndex& index)  /*테이블 뷰 클릭 시 해당되는 데이터들을 에디터에 호출*/
{
    Q_UNUSED(index);
    QModelIndex tIndex = ui->tableView->currentIndex();             /*테이블 뷰에 인덱스*/
    QString ID = tIndex.sibling(tIndex.row(), 0).data().toString(); /*테이블의 id를 에디트로 가져오기*/
    QString name = tIndex.sibling(tIndex.row(), 1).data().toString();   /*테이블의 name을 에디트로 가져오기*/
    QString phone = tIndex.sibling(tIndex.row(), 2).data().toString();  /*테이블의 전화번호를 에디트로 가져오기*/
    QString address = tIndex.sibling(tIndex.row(), 3).data().toString();    /*테이블의 이메일을 에디트로 가져오기*/
    //qDebug() << ID << " " << name << " " << phone << " "  << address;

    /*현존하는 ui에디트에 출력*/
    ui->CIDLineEdit->setText(ID);
    ui->CNameLineEdit->setText(name);
    ui->CPhoneLineEdit->setText(phone);
    ui->CEmailLineEdit->setText(address);
    emit ClientAdded(name);

    ui->toolBox->setCurrentIndex(0);
}


