#include "productmanager.h"
#include "ui_productmanager.h"

#include <QFile>
#include <QMenu>

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
#include <QSqlDatabase>


/*고객 관리 프로파일과 거의 동일한 기능을 수행*/
ProductManager::ProductManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductManager)
{
    ui->setupUi(this);
    ui->toolBox->setCurrentIndex(0);

    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem())); /*제거하고자 하는 아이템 액션 수행*/

    menu = new QMenu;
    menu->addAction(removeAction);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextItem(QPoint)));           /*컨택스트 메뉴 호출 커넥트*/

    /*데이터 보내기*/
    connect(ui->PNameLineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(ProductAdded(QString)));               /*상품의 이름 데이터 전송*/

    connect(ui->PPriceLineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(ProductPrices(QString)));              /*상품의 가격 데이터 전송*/

    /*데이터 베이스의 데이터들을 불러오기*/

    QSqlDatabase ProductDB = QSqlDatabase::addDatabase("QODBC", "productConnection");
    /*추가하려는 데이터베이스 종류는 QODBC(Qt Oracle DataBase)*/
    ProductDB.setDatabaseName("Oracle11g");            /*데이터베이스 이름*/
    ProductDB.setUserName("ProjectDB");                /*데이터 베이스 계정 명*/
    ProductDB.setPassword("1234");                     /*데이터 베이스 비밀번호*/

    if (ProductDB.open()) {
        ProductQuery = new QSqlQuery(ProductDB);        /*상품 데이터베이스 쿼리문*/
        ProductModel = new QSqlTableModel(this, ProductDB);   /*릴레이션 테이블 동기화*/
        ProductModel->setTable("GOODS");                /*GOODS 테이블 탐색*/
        ProductModel->select();

        /*고객 데이터베이스 출력 쿼리문*/
        ProductModel->setHeaderData(0, Qt::Horizontal, QObject::tr("g_id"));
        ProductModel->setHeaderData(1, Qt::Horizontal, QObject::tr("g_name"));
        ProductModel->setHeaderData(2, Qt::Horizontal, QObject::tr("g_com"));
        ProductModel->setHeaderData(3, Qt::Horizontal, QObject::tr("g_price"));

        ui->tableView->setModel(ProductModel);
        ui->tableView->resizeColumnsToContents();       /*데이터 사이즈에 맞게 열을 정렬*/
    }

    SearchModel = new QStandardItemModel(0, 4);                         /*행렬중 row = 0, column = 4로 초기화*/
    SearchModel->setHeaderData(0, Qt::Horizontal, tr("ID"));            /*1번째 column 이름을 ID*/
    SearchModel->setHeaderData(1, Qt::Horizontal, tr("Name"));          /*2번째 column 이름을 Name*/
    SearchModel->setHeaderData(2, Qt::Horizontal, tr("Company"));  /*3번째 column 이름을 Phone Number*/
    SearchModel->setHeaderData(3, Qt::Horizontal, tr("Price"));         /*4번째 column 이름을 Email로 설정*/
    ui->TBSearchView->setModel(SearchModel);                         /*테이블 뷰 위젯에 SearchModel 추가*/
}

ProductManager::~ProductManager()
{
    delete ui;

    QSqlDatabase ProductDB = QSqlDatabase::database("productConnection");  /*db는 현재 가지고 있는 데이터 베이스*/
    if(ProductDB.isOpen())                     /*데이터 베이스가 열려있다면*/
    {
        ProductModel->submitAll();
        /*보류 중인 모든 변경 사항을 제출하고 성공하면 true를 반환합니다.
         * 오류 시 false를 반환하고 lastError()를 사용하여 자세한 오류 정보를 얻을 수 있습니다.*/

        ProductDB.close();
        QSqlDatabase::removeDatabase("productConnection");     /*할당되어 있는 QODBC를 제거*/
    }
}

int ProductManager::makeID()        /*상품의 아이디 자동 할당*/
{
    if(ProductModel->rowCount() == 0){  /*상품의 데이터베이스 정보에 어떠한 정보도 들어 있지 않다면*/
        return 1;               /*ID를 1부터 시작*/
    }else{
        int id = ui->PIDLineEdit->text().toInt();    /**/
        /*상품정보중 최근 정보의 id에 접근*/
        return ++id;                /*최근 아이디에 + 1 연산*/
    }
}

/*컨텍스트 메뉴를 마우스 우클릭 시 지정된 위치에서 메뉴호출*/
void ProductManager::showContextItem(const QPoint& pos)
{
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
    if(ui->tableView->indexAt(pos).isValid())
        menu->exec(globalPos);
}

void ProductManager::removeItem()   /*상품 데이터의 데이터를 삭제하는 함수*/
{
    int index = ui->PIDLineEdit->text().toInt();/*상품 테이블 데이터 인덱싱*/
    if(index){    /*해당 인덱스의 값이 존재하면*/
        ProductQuery->exec(QString("CALL DELETE_GOOD(%1)").arg(index));    /*해당하는 인덱스 데이터를 지우는 프로시져 호출*/
        ProductModel->select();      /*해당 테이블 호출*/
        //ui->tableView->setModel(ClientModel);   /*테이블 모델 셋*/
        ui->tableView->update();
        ui->tableView->resizeColumnsToContents();       /*데이터 사이즈에 맞게 열을 정렬*/
    }
}


/*상품의 정보 입력*/
void ProductManager::on_InputButton_clicked()       /*상품의 정보를 데이터 베이스에 추가*/
{
    QString name, company;                  /*상품정보의 데이터 4개 변수 할당*/
    int price;
    int id = makeID( );                     /*아이디는 자동으로 생성*/
    ui->PIDLineEdit->setText(QString::number(id));      /*아이디 라인에디트*/
    name = ui->PNameLineEdit->text();                   /*이름 라인에디트*/
    company = ui->PCompanyLineEdit->text();             /*회사 라인에디트*/
    price = ui->PPriceLineEdit->text().toInt();         /*상품 가격 라인 에디트*/
    if(id && name.length() && company.length() && price) {    /*이름, 회사, 가격 정보를 모두 입력한 경우*/
        /*오라클 내의 프로시져를 사용*/
        ProductQuery->exec(QString("CALL INSERT_GOOD(%1, '%2', '%3', %4)")
                           .arg(id).arg(name).arg(company).arg(price));
        ProductModel->select();                     /*릴레이션 테이블 호출*/
        ui->tableView->setModel(ProductModel);      /*테이블 뷰에 띄우기*/
        //ProductModel->QSqlQueryModel::setQuery("SELECT * FROM GOODS ORDER BY G_ID");
    }
}

/*상품의 정보를 입력하다가 에디터에 있는데이터를 모두 지우는 경우*/
void ProductManager::on_CancelButton_clicked()
{
    ui->PIDLineEdit->setText("");
    ui->PNameLineEdit->setText("");
    ui->PCompanyLineEdit->setText("");
    ui->PPriceLineEdit->setText("");
}

/*상품의 정보를 수정하는 경우*/
void ProductManager::on_ModifyButton_clicked()          /*상품 데이터베이스 데이터 수정*/
{
    QModelIndex tIndex = ui->tableView->currentIndex(); /*테이블 뷰 인덱스 할당*/
    if(tIndex.isValid()) {                      /*인덱스가 존재 한다면*/
        int ID = ui->PIDLineEdit->text().toInt();   /*정수형 데이터(id)*/
        QString name = ui->PNameLineEdit->text();   /*QString 데이터*/
        QString company = ui->PCompanyLineEdit->text();
        int price = ui->PPriceLineEdit->text().toInt();
        if(name.length() && company.length() && price)  /*정보를 모두 입력해야 데이터베이스에 데이터를 할당*/
        {

            ProductQuery->exec(QString("CALL UPDATE_GOOD(%1, '%2', '%3', %4)")
                               .arg(ID).arg(name).arg(company).arg(price));

            ProductModel->select();
            ui->tableView->setModel(ProductModel);      /*수정된 데이터 베이스를 호출*/
            ui->tableView->resizeColumnsToContents();       /*데이터 사이즈에 맞게 열을 정렬*/
        }
    }
}

/*검색 버튼 클릭시 해당되는 컬럼과 이름의 정보를 출력*/
void ProductManager::on_Search_clicked()
{
    SearchModel->clear();                                          /*SearhModel 초기화*/
    int i = ui->SearchComboBox->currentIndex();                    /*콤보박스에 해당하는 인덱스 변수 선언*/
    auto flag = (i) ? Qt::MatchCaseSensitive|Qt::MatchContains     /*검색 플래그 매칭 조건*/
                    : Qt::MatchCaseSensitive;
    QModelIndexList indexs = ProductModel->match(ProductModel->index(0, i),   /*match() 파라미터에 따른 검색 리스트 인덱스 나열*/
             Qt::EditRole, ui->SearchLineEdit->text(), -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexs){
        int id = ProductModel->data(ix.siblingAtColumn(0)).toInt(); //해당되는 열을 출력(id에 해당하는 모든 정보)
        QString name = ProductModel->data(ix.siblingAtColumn(1)).toString(); //name에 해당되는 열을 출력
        QString company= ProductModel->data(ix.siblingAtColumn(2)).toString(); //number에 해당되는 열을 출력
        int price = ProductModel->data(ix.siblingAtColumn(3)).toInt(); //address에 해당되는 열을 출력
        QStringList strings;
        strings << QString::number(id) << name << company << QString::number(price);          //검색된 행에 아이디, 이름, 전화번호, 이메일을 strings에 순서대로 저장

        QList<QStandardItem *> items;                                       /*QStandardItme을 상속한 리스트 아이템 변수를 선언*/
        for(int i = 0; i < 4; i++){
            items.append(new QStandardItem(strings.at(i)));                 /*4번째 컬럼까지 데이터를 append*/
        }

        SearchModel->appendRow(items);                                      /*1개의 행 내용을 전부 출력*/
        SearchModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        SearchModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        SearchModel->setHeaderData(2, Qt::Horizontal, tr("Company"));
        SearchModel->setHeaderData(3, Qt::Horizontal, tr("Price"));


        ui->TBSearchView->resizeColumnsToContents();                     /*입력된 데이터의 크기 만큼 컬럼을 조정*/
    }
}

/*테이블 뷰 검색 리스트의 해당 아이템을 클릭하면 상품의 이름과 가격을 에디터에 연결*/
void ProductManager::on_TBSearchView_clicked(const QModelIndex &index)
{
    QString name = index.sibling(index.row(), 1).data().toString();
    int price = index.sibling(index.row(), 3).data().toInt();
    ui->PNameLineEdit->setText(name);
    ui->PPriceLineEdit->setText(QString::number(price));
}

void ProductManager::on_tableView_clicked(const QModelIndex &index) /*테이블 뷰 클릭 시 해당되는 데이터들을 에디터에 호출*/
{
    QString ID = index.sibling(index.row(), 0).data().toString();   /*테이블의 id를 할당*/
    QString name = index.sibling(index.row(), 1).data().toString(); /*테이블의 이름을 할당*/
    QString com = index.sibling(index.row(), 2).data().toString();  /*테이블의 전화번호를 할당*/
    QString price = index.sibling(index.row(), 3).data().toString();/*테이블의 price를 할당*/
    /*현존하는 ui 에디트에 데이터들을 출력*/
    ui->PIDLineEdit->setText(ID);
    ui->PNameLineEdit->setText(name);
    ui->PCompanyLineEdit->setText(com);
    ui->PPriceLineEdit->setText(price);

    ui->toolBox->setCurrentIndex(0);    /*테이블 뷰 데이터를 클릭시 INPUT 툴 박스로 위치 조정*/
}


void ProductManager::on_RecentButton_clicked()      /*아이디 중 최대값을 출력*/
{
    ProductQuery->prepare("SELECT MAX(G_ID) FROM GOODS;");/*현존하는 아이디중 최댓값을 출력하는 쿼리문*/
    ProductQuery->exec();            /*쿼리문 실행*/
    while (ProductQuery->next()) {   /*쿼리문 진행*/
        qDebug() << ProductQuery->value(0).toInt();  /*쿼리값 디버깅*/
        ui->RecentLineEdit->setText(QString::number(ProductQuery->value(0).toInt()));    /*아이디 라인 에디트체 자동 할당 실행*/
        ui->PIDLineEdit->setText(QString::number(ProductQuery->value(0).toInt()));       /*표시 라인 에디트에 자동 할당*/
        //ClientModel->select();
    }
}



