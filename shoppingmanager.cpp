#include "shoppingmanager.h"
#include "ui_shoppingmanager.h"

#include <QList>
#include <QStringList>
#include <QString>
#include <QFile>
#include <QMenu>

#include <QDateTimeEdit>
#include <QDate>

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



ShoppingManager::ShoppingManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShoppingManager)
{
    ui->setupUi(this);
    ui->toolBox->setCurrentIndex(0);

    QAction* removeAction = new QAction(tr("&Remove"));         /*제거 액션 할당*/
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem())); /*마우스 우클릭 시 커넥트 하는 함수*/

    menu = new QMenu;
    menu->addAction(removeAction);          /*메뉴 할당*/
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);    /*구매정보 리스트에만 나오는 컨텍스트 메뉴 선언*/
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));                   /*컨텍스트를 띄울수 있는 커넥트 함수*/

    QDate* date = new QDate;
    ui->SDateLineEdit->setDate(date->currentDate());

    QSqlDatabase ShoppingDB = QSqlDatabase::addDatabase("QODBC", "shoppingConnection");
    /*추가하려는 데이터베이스 종류는 QODBC(Qt Oracle Database)*/
    ShoppingDB.setDatabaseName("Oracle11g");    /*연결 데이터베이스 이름*/
    ShoppingDB.setUserName("ProjectDB");        /*데이터 베이스 계정명*/
    ShoppingDB.setPassword("1234");             /*데이터 베이스 비밀번호*/

    if(ShoppingDB.open()){
        ShoppingQuery = new QSqlQuery(ShoppingDB);      /*작동할 쿼리문을 해당 데이터베이스에 할당*/
        ShoppingModel = new QSqlTableModel(this, ShoppingDB);   /*테이블 모델을 사용*/
        ShoppingModel->setTable("ORDERS");              /*테이블 중 ORDERS 테이블 사용*/
        ShoppingModel->select();                        /*테이블 호출*/

        ShoppingModel->setHeaderData(0, Qt::Horizontal, QObject::tr("o_id"));
        ShoppingModel->setHeaderData(1, Qt::Horizontal, QObject::tr("c_name"));
        ShoppingModel->setHeaderData(2, Qt::Horizontal, QObject::tr("g_name"));
        ShoppingModel->setHeaderData(3, Qt::Horizontal, QObject::tr("o_date"));
        ShoppingModel->setHeaderData(4, Qt::Horizontal, QObject::tr("o_quan"));
        ShoppingModel->setHeaderData(5, Qt::Horizontal, QObject::tr("o_allprice"));

        ui->tableView->setModel(ShoppingModel);
        ui->tableView->resizeColumnsToContents();       /*데이터 사이즈에 맞게 열을 정렬*/
    }

    SearchModel = new QStandardItemModel(0, 6);                         /*행렬중 row = 0, column = 4로 초기화*/
    SearchModel->setHeaderData(0, Qt::Horizontal, tr("ID"));            /*1번째 column 이름을 ID*/
    SearchModel->setHeaderData(1, Qt::Horizontal, tr("ClientName"));    /*2번째 column 이름을 ClientName*/
    SearchModel->setHeaderData(2, Qt::Horizontal, tr("ProductName"));  /*3번째 column 이름을 ProductNumber*/
    SearchModel->setHeaderData(3, Qt::Horizontal, tr("Date"));         /*4번째 column 이름을 Date로 설정*/
    SearchModel->setHeaderData(4, Qt::Horizontal, tr("Quan"));          /*5번째 column 이름을 Quan으로 설정*/
    SearchModel->setHeaderData(5, Qt::Horizontal, tr("AllPrice"));     /*6번째 column 이름을 AllPrice로 설정*/
    ui->TBSearchView->setModel(SearchModel);                         /*테이블 뷰 위젯에 SearchModel 추가*/
}

ShoppingManager::~ShoppingManager()
{
    delete ui;

    QSqlDatabase ShoppingDB = QSqlDatabase::database("shoppingConnection");/*구매 정보 데이터 베이스 변수 선언*/
    if(ShoppingDB.isOpen())
    {
        ShoppingModel->submitAll();
        /*보류 중인 모든 변경 사항을 제출하고 성공하면 true를 반환합니다.
         * 오류 시 false를 반환하고 lastError()를 사용하여 자세한 오류 정보를 얻을 수 있습니다.*/
        ShoppingDB.close();
        QSqlDatabase::removeDatabase("shoppingConnection"); /*할당된 데이터베이스를 제거*/
    }
}

void ShoppingManager::CreceiveData( QString str)
{
    ui->ClientLineEdit->setText(str);           /*고객의 성함을 받는 슬롯*/
}

void ShoppingManager::PreceiveData(QString str)
{
    ui->ProductLineEdit->setText(str);          /*상품의 이름을 받는 슬롯*/
}

void ShoppingManager::PreceivePrice(QString price)
{
    ui->SAllPriceLineEdit->setText(price);      /*상품의 가격을 받는 슬롯*/
}

int ShoppingManager::makeId( )          /*구매 정보 아이디 자동 할당*/
{
    if(ShoppingModel->rowCount() == 0){ //구매정보데이터베이스에 데이터가 없으면
        return 1;               //1을 반환
    }else{
        int id = ui->SIDLineEdit->text().toInt();
        return ++id;            /*선택한 에디트 아이디로 자동 +1 연산*/
    }
}


void ShoppingManager::showContextMenu(const QPoint& pos)        /*컨택스트 메뉴 슬롯*/
{
    QPoint globalPos = ui->tableView->mapToGlobal(pos);    /*매개인자 우클릭 시 위치대로 메뉴를 생성하게 하는 방향 선언*/
    if(ui->tableView->indexAt(pos).isValid())
        menu->exec(globalPos);          /*메뉴 실행*/
}

void ShoppingManager::removeItem()              /*우클릭 시 삭제할 정보를 리스트에서 지워주는 함수*/
{
    int index = ui->SIDLineEdit->text().toInt();     /*할당된 아이템 선언*/
    if(index){                      //인덱스 값이 존재하는 경우
        ShoppingQuery->exec(QString("CALL DELETE_ORDERS(%1)").arg(index));
        /*해당하는 인덱스 데이터를 지우는 프로시져*/
        ShoppingModel->select();        /*해당 테이블 select 호출*/
        ui->tableView->update();        /*해당 테이블 업데이트*/
    }
}

void ShoppingManager::on_InputButton_clicked()      /*구매 정보 데이터 베이스에 데이터 추가*/
{
    QString client, product, date;          /*구매 정보의 데이터 6개 변수 할당*/
    int id = makeId( );
    int quan = 0, allprice = 0;

    ui->SIDLineEdit->setText(QString::number(id));  /*아이디 라인 에디트*/
    client = ui->ClientLineEdit->text();            /*고객 성함 라인에디트*/
    product = ui->ProductLineEdit->text();          /*상품 이름 라인에디트*/
    date = ui->SDateLineEdit->date().toString("yyyy-MM-dd");    /*날짜 정보 라인에디트*/
    quan = ui->SQuanLineEdit->text().toInt();                   /*구매 정보 수량 라인 에디트*/
    allprice = ui->SAllPriceLineEdit->text().toInt() * quan;        /*총가격은 상품의 가격과 수량의 곱으로 총가격을 표시*/
    /*위의 데이터들은 텍스트 파일로 저장*/

    if(client.length() && product.length() && date.length() && quan) { /*고객의 성함과 상품의 이름이 있는경우에만 정보 추가*/
        /*오라클 내의 프로시져를 사용*/
        ShoppingQuery->exec(QString("CALL INSERT_ORDERS(%1, '%2', '%3', %4)")
                            .arg(id).arg(client).arg(product).arg(quan));
        ShoppingModel->select();                    /*릴레이션 테이블 호출*/
        ui->tableView->setModel(ShoppingModel);     /*테이블 뷰에 띄우기*/
        ui->tableView->resizeColumnsToContents();       /*데이터 사이즈에 맞게 열을 정렬*/
    }
}


void ShoppingManager::on_CancelButton_clicked()     /*구매 정보를 입력하다가 잘못 입력한 것 같으면 라인에디트를 모두 지움*/
{
    ui->SIDLineEdit->setText("");
    ui->ClientLineEdit->setText("");
    ui->ProductLineEdit->setText("");
    //ui->SDateLineEdit->setText("");
    ui->SQuanLineEdit->setText("");
    ui->SAllPriceLineEdit->setText("");
}


void ShoppingManager::on_ModifyButton_clicked()     /*구매 정보를 수정했을 경우 재입력 되는 버튼 함수*/
{
    QModelIndex tIndex = ui->tableView->currentIndex(); /*구매 정보 테이블 뷰 인덱스 할당*/
    if(tIndex.isValid()) {                      /*데이터 베이스 인덱스에 데이터가 존재 한다면*/
        int ID = ui->SIDLineEdit->text().toInt();       /*정수형 데이터(id)*/
        QString clientname = ui->ClientLineEdit->text();      /*QString 데이터*/
        QString productname = ui->ProductLineEdit->text();
        int quan = ui->SQuanLineEdit->text().toInt();

        if(clientname.length() && productname.length() && quan) /*수정할 정보를 모두 입력 했다면*/
        {
            ShoppingQuery->exec(QString("CALL UPDATE_ORDERS(%1, '%2', '%3', %4)")
                                .arg(ID).arg(clientname).arg(productname).arg(quan));
            /*구매 정보 업데이트 프로시져 호출*/
            ShoppingModel->select();    /*테이블 호출*/
            ui->tableView->update();    /*테이블 업데이트*/
            ui->tableView->resizeColumnsToContents();       /*데이터 사이즈에 맞게 열을 정렬*/
        }
    }
}


void ShoppingManager::on_SearchButton_clicked()         /*검색 버튼을 눌렀을 시 탐색하여 해당 데이터의 정보를 호출*/
{
    SearchModel->clear();                                          /*SearhModel 초기화*/
    int i = ui->comboBox->currentIndex();                    /*콤보박스에 해당하는 인덱스 변수 선언*/
    auto flag = (i) ? Qt::MatchCaseSensitive|Qt::MatchContains     /*검색 플래그 매칭 조건*/
                    : Qt::MatchCaseSensitive;
    QModelIndexList indexs = ShoppingModel->match(ShoppingModel->index(0, i),   /*match() 파라미터에 따른 검색 리스트 인덱스 나열*/
             Qt::EditRole, ui->SearchLineEdit->text(), -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexs){
        int id = ShoppingModel->data(ix.siblingAtColumn(0)).toInt(); //해당되는 열을 출력(id에 해당하는 모든 정보)
        QString clientname = ShoppingModel->data(ix.siblingAtColumn(1)).toString(); //clientname에 해당되는 열을 출력
        QString productname = ShoppingModel->data(ix.siblingAtColumn(2)).toString();//productname에 해당되는 열을 출력
        QString date= ShoppingModel->data(ix.siblingAtColumn(3)).toString(); //date에 해당되는 열을 출력
        int quan = ShoppingModel->data(ix.siblingAtColumn(4)).toInt();      //quan에 해당되는 열을 출력
        int allprice = ShoppingModel->data(ix.siblingAtColumn(5)).toInt();  //allprice에 해당되는 열을 출력
        QStringList strings;
        strings << QString::number(id) << clientname << productname
                << date << QString::number(quan) << QString::number(allprice);          //검색된 행에 아이디, 이름, 전화번호, 이메일을 strings에 순서대로 저장

        QList<QStandardItem *> items;                                       /*QStandardItme을 상속한 리스트 아이템 변수를 선언*/
        for(int i = 0; i < 6; i++){
            items.append(new QStandardItem(strings.at(i)));                 /*4번째 컬럼까지 데이터를 append*/
        }

        SearchModel->appendRow(items);                                      /*1개의 행 내용을 전부 출력*/
        SearchModel->setHeaderData(0, Qt::Horizontal, tr("ID"));            /*1번째 column 이름을 ID*/
        SearchModel->setHeaderData(1, Qt::Horizontal, tr("ClientName"));    /*2번째 column 이름을 ClientName*/
        SearchModel->setHeaderData(2, Qt::Horizontal, tr("ProductName"));  /*3번째 column 이름을 ProductNumber*/
        SearchModel->setHeaderData(3, Qt::Horizontal, tr("Date"));         /*4번째 column 이름을 Date로 설정*/
        SearchModel->setHeaderData(4, Qt::Horizontal, tr("Quan"));          /*5번째 column 이름을 Quan으로 설정*/
        SearchModel->setHeaderData(5, Qt::Horizontal, tr("AllPrice"));     /*6번째 column 이름을 AllPrice로 설정*/

        ui->TBSearchView->resizeColumnsToContents();                     /*입력된 데이터의 크기 만큼 컬럼을 조정*/
    }
}

void ShoppingManager::on_tableView_clicked(const QModelIndex &index) //테이블 뷰 클릭 함수
{
    QString ID = index.sibling(index.row(), 0).data().toString();   //테이블 id변수 선언
    QString clientname = index.sibling(index.row(), 1).data().toString();   /*고객 성함 변수*/
    QString proccudtname = index.sibling(index.row(), 2).data().toString(); /*상품 이름 변수 선언*/
    QString date = index.sibling(index.row(), 3).data().toString();     /*구매 날짜 변수 선언*/
    QString quan = index.sibling(index.row(), 4).data().toString();     /*구매 수량 변수 선언*/
    QString price = index.sibling(index.row(), 5).data().toString();    /*구매 가격 변수 선언*/

    /*현존하는 ui에디트에 데이터들을 입력*/
    ui->SIDLineEdit->setText(ID);
    ui->ClientLineEdit->clear();                /*고객의 성함 에디터의 데이터를 지우기*/
    ui->ClientLineEdit->setPlaceholderText("choose Client Name");  /*수정을 원하는 고객의 성함을 선택하라고 알림*/
    ui->ProductLineEdit->clear();               /*상품의 이름 에디터의 데이터를 지우기*/
    ui->ProductLineEdit->setPlaceholderText("choose Product Name"); /*수정을 원하는 상품의 이름을 선택하라 알림*/
    ui->SDateLineEdit->setDate(QDate::fromString(date, "yyyy-MM-dd"));          /*구매정보의 날짜를 알려주는 에디터 라인*/
    ui->SQuanLineEdit->setText(quan);          /*상품의 수량을 알려주는 에디터 라인*/
    //ui->SAllPriceLineEdit->setText(item->text(5));
    ui->SAllPriceLineEdit->setText("");                 /*상품의 가격은 상품의 이름을 선택할 시 자동으로 채워지니 빈텍스트를 출력*/
    ui->toolBox->setCurrentIndex(0);                /*아이템 클릭시 toolbox를 inputbox로 조정*/
}


void ShoppingManager::on_RecentButton_clicked()
{
    ShoppingQuery->prepare("SELECT MAX(O_ID) FROM ORDERS;");/*현존하는 아이디중 최댓값을 출력하는 쿼리문*/
    ShoppingQuery->exec();            /*쿼리문 실행*/
    while (ShoppingQuery->next()) {   /*쿼리문 진행*/
        qDebug() << ShoppingQuery->value(0).toInt();  /*쿼리값 디버깅*/
        ui->RecentLineEdit->setText(QString::number(ShoppingQuery->value(0).toInt()));    /*아이디 라인 에디트체 자동 할당 실행*/
        ui->SIDLineEdit->setText(QString::number(ShoppingQuery->value(0).toInt()));       /*표시 라인 에디트에 자동 할당*/
        //ClientModel->select();
    }
}
