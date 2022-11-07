#include "shoppingmanager.h"
#include "ui_shoppingmanager.h"
#include "shopping.h"

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
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>



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
    ui->ShoppingTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);    /*구매정보 리스트에만 나오는 컨텍스트 메뉴 선언*/
    connect(ui->ShoppingTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));                   /*컨텍스트를 띄울수 있는 커넥트 함수*/

    /*파일 불러오기*/
    QFile file("shoppinglist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {           /*텍스트의 흐름을 읽어내는데 끝까지 읽어냄*/
        QString line = in.readLine();
        QList<QString> row = line.split(", "); // , 특수 문자가 보일때마다 분류
        if(row.size()) {
            int id = row[0].toInt();
            int quan = row[4].toInt();
            int price = row[5].toInt();
            Shopping* c = new Shopping(id, row[1], row[2], row[3], quan, price);
            ui->ShoppingTreeWidget->addTopLevelItem(c); /*텍스트에 저장된 데이터들을 리스트에 1행씩 주입*/
            shoppingList.insert(id, c);
        }
    }
    file.close( );
    /*파일 불러오기 끝*/
    QDate* date = new QDate;
    ui->SDateLineEdit->setDate(date->currentDate());


    //ui->SDateLineEdit->setPlaceholderText("press yyyy-mm-dd or Enter");     /*날짜 입력 형식을 표시*/
//    QModelIndex i = ui->ShoppingTreeWidget->currentIndex();
//    ui->ShoppingTreeWidget->setRowHidden(0, i, true);
//    ui->ShoppingTreeWidget->setRowHidden(1, i, true);
}

ShoppingManager::~ShoppingManager()
{
    delete ui;

    /*파일 저장*/
    QFile file("shoppinglist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    /*나중에 콤보박스로 데이터를 넣을 때 이방식을 적용해보자*/
    QTextStream out(&file);
    //out << "shoppingID, clientName, productName, Date, quan, AllPrice\n";
    for (const auto& v : shoppingList) {
        Shopping* s = v;
        out << s->getId() << ", " << s->getClientName() << ", ";
        out << s->getProductName() << ", ";
        out << s->getDate() << ", ";
        out << s->getquan() << ", ";
        out << s->getAllPrice() << "\n";        /*데이터들을 ,로 구분짓고 1행의 데이터 주입이 마치면 \n*/
    }
    file.close( );
}

bool ShoppingManager::shoppingDataConnection( )
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

int ShoppingManager::makeId( )          /*아이디 자동 할당*/
{
    if(shoppingList.size( ) == 0) {
        return 1;
    } else {
        auto id = shoppingList.lastKey();
        return ++id;
    }
}


void ShoppingManager::showContextMenu(const QPoint& pos)        /*컨택스트 메뉴 슬롯*/
{
    QPoint globalPos = ui->ShoppingTreeWidget->mapToGlobal(pos);    /*매개인자 우클릭 시 위치대로 메뉴를 생성하게 하는 방향 선언*/
    menu->exec(globalPos);          /*메뉴 실행*/
}

void ShoppingManager::removeItem()              /*우클릭 시 삭제할 정보를 리스트에서 지워주는 함수*/
{
    QTreeWidgetItem* item = ui->ShoppingTreeWidget->currentItem();      /*할당된 아이템 선언*/
    if(item !=  nullptr)
    {
        shoppingList.remove(item->text(0).toInt());             /*해당하는 ID를 제거(ID에 속한 데이터들은 지워짐)*/
        ui->ShoppingTreeWidget->takeTopLevelItem
                (ui->ShoppingTreeWidget->indexOfTopLevelItem(item)); /*구매 정보 리스트에서 제거됨*/
        ui->ShoppingTreeWidget->update();                   /*이후 구매정보 최신화*/
    }

    if (!shoppingDataConnection( )) return;           /*구매 정보 데이터베이스에 접근하지 못한 경우*/
    QSqlQueryModel queryModel;
    queryModel.setQuery(QString("CALL DELETE_ORDERS(%1)").arg(item->text(0).toInt()));     /*데이터 베이스에서 삭제할 구매 정보*/
}

/*수정하고자 하는 구매정보 리스트를 클릭하여 하단과 같이 해당 정보를 입력*/
void ShoppingManager::on_ShoppingTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->SIDLineEdit->setText(item->text(0));
    ui->ClientLineEdit->clear();                /*고객의 성함 에디터의 데이터를 지우기*/
    ui->ClientLineEdit->setPlaceholderText("choose Client Name");  /*수정을 원하는 고객의 성함을 선택하라고 알림*/
    ui->ProductLineEdit->clear();               /*상품의 이름 에디터의 데이터를 지우기*/
    ui->ProductLineEdit->setPlaceholderText("choose Product Name"); /*수정을 원하는 상품의 이름을 선택하라 알림*/
    ui->SDateLineEdit->setDate(QDate::fromString(item->text(3), "yyyy-MM-dd"));          /*구매정보의 날짜를 알려주는 에디터 라인*/
    ui->SQuanLineEdit->setText(item->text(4));          /*상품의 수량을 알려주는 에디터 라인*/
    //ui->SAllPriceLineEdit->setText(item->text(5));
    ui->SAllPriceLineEdit->setText("");                 /*상품의 가격은 상품의 이름을 선택할 시 자동으로 채워지니 빈텍스트를 출력*/
    ui->toolBox->setCurrentIndex(0);                /*아이템 클릭시 toolbox를 inputbox로 조정*/
}


void ShoppingManager::on_InputButton_clicked()
{
    QString client, product, date;
    int id = makeId( );
    int quan, allprice = 0;
    client = ui->ClientLineEdit->text();
    product = ui->ProductLineEdit->text();
    date = ui->SDateLineEdit->date().toString("yyyy-MM-dd");
    quan = ui->SQuanLineEdit->text().toInt();
    allprice = ui->SAllPriceLineEdit->text().toInt() * quan;        /*총가격은 상품의 가격과 수량의 곱으로 총가격을 표시*/
    /*위의 데이터들은 텍스트 파일로 저장*/

    if(client.length() && product.length() && date.length() && quan) {               /*고객의 성함과 상품의 이름이 있는경우에만 정보 추가*/
        Shopping* s = new Shopping(id, client, product, date, quan, allprice);
        shoppingList.insert(id, s);
        ui->ShoppingTreeWidget->addTopLevelItem(s);

        /*구매 정보 데이터베이스가 연결되지 않았을 경우*/
        if (!shoppingDataConnection( )) return;

        QSqlQueryModel queryModel;
        queryModel.setQuery(QString("CALL INSERT_ORDERS(%1, '%2', '%3', %4)").arg(id).arg(client).arg(product).arg(quan));
        /*구매 정보데이터 베이스로 들어갈 데이터*/
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
    QTreeWidgetItem* item = ui->ShoppingTreeWidget->currentItem();
    if(item != nullptr) {
        int key = item->text(0).toInt();
        Shopping* s = shoppingList[key];
        QString client, product, date;
        int quan, price;
        client = ui->ClientLineEdit->text();
        product = ui->ProductLineEdit->text();
        date = ui->SDateLineEdit->date().toString("yyyy-MM-dd");        /*날짜 업데이트*/
        quan = ui->SQuanLineEdit->text().toInt();
        price = ui->SAllPriceLineEdit->text().toInt() * quan;

        if(client.length() && product.length() && date.length() && quan)    /*client, product, shopping 모두 에디터에 데이터가 존재시 데이터베이스까지 업데이트*/
        {
            /*해당 데이터들이 가지고 있는 리스트의 1행을 모두 수정*/
            s->setClientName(client);
            s->setProductName(product);
            s->setDate(date);
            s->setquan(quan);
            s->setAllPrice(price);
            shoppingList[key] = s;

            if (!shoppingDataConnection( )) return;           /*구매 정보 데이터베이스에 접근하지 못한 경우*/
            QSqlQueryModel queryModel;
            queryModel.setQuery(QString("CALL UPDATE_ORDERS(%1, '%2', '%3', %4)")
                                .arg(key).arg(client).arg(product).arg(quan));     /*데이터 베이스에서 수정할 고객 정보*/
        }
    }
}


void ShoppingManager::on_SearchButton_clicked()         /*검색 버튼을 눌렀을 시 탐색하여 해당 데이터의 정보를 호출*/
{
    ui->SearchTreeWidget->clear();
    int i = ui->comboBox->currentIndex();
    auto flag = (i)? Qt::MatchCaseSensitive|Qt::MatchContains
                   : Qt::MatchCaseSensitive;
    {
        auto items = ui->ShoppingTreeWidget->findItems
                (ui->SearchLineEdit->text(), flag, i);

        foreach(auto i, items) {
            Shopping* s = static_cast<Shopping*>(i);
            int id = s->getId();
            QString client = s->getClientName();
            QString product = s->getProductName();
            QString date = s->getDate();
            int quan = s->getquan();
            int price = s->getAllPrice();
            Shopping* item = new Shopping(id, client, product, date,
                                        quan, price);
            ui->SearchTreeWidget->addTopLevelItem(item);
        }
    }
}


void ShoppingManager::on_SDateLineEdit_returnPressed()          /*엔터키를 입력시 자동으로 금일 날짜를 호출하는 함수*/
{
    QDateTimeEdit* datetimeedit = new QDateTimeEdit(QDate::currentDate(), 0);   /*현재 날짜를 yyyy-mm-dd형태의 라인에디트로 선언*/
    //datetimeedit->setCalendarPopup(true);
    //ui->SDateLineEdit->setText(datetimeedit->text());       /*텍스트 형태의 데이터를 날짜 입력 라인에디터에 호출*/
}

