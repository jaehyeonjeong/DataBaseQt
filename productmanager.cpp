#include "productmanager.h"
#include "ui_productmanager.h"
#include "product.h"

#include <QFile>
#include <QMenu>

/*Oracle SQL 연동을 위한 헤더*/
#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


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
    ui->ProductTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->ProductTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextItem(QPoint)));           /*컨택스트 메뉴 호출 커넥트*/

    /*데이터 보내기*/
    connect(ui->PNameLineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(ProductAdded(QString)));               /*상품의 이름 데이터 전송*/

    connect(ui->PPriceLineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(ProductPrices(QString)));              /*상품의 가격 데이터 전송*/


    /*파일 불러오기*/
    QFile file("productList.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");
        if(row.size()) {
            int id = row[0].toInt();
            int price = row[3].toInt();
            Product* c = new Product(id, row[1], row[2], price);
            ui->ProductTreeWidget->addTopLevelItem(c);      /*살품의 정보(아이디, 이름, 회사, 가격) 주입*/
            productList.insert(id, c);

            emit ProductAdded(row[1]);
        }
    }
    file.close( );
//    QModelIndex i = ui->ProductTreeWidget->currentIndex();
//    ui->ProductTreeWidget->setRowHidden(0, i, true);
//    ui->ProductTreeWidget->setRowHidden(1, i, true);
}

ProductManager::~ProductManager()
{
    delete ui;

    /*파일 저장*/
    QFile file("productList.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);             /*파일을 읽을 수 있는 데이터의 흐름*/
    //out << "productID, productName, productCompany, productPrice\n";
    for (const auto& v : productList) {     /*리스트로 정열된 상품의 정보 나열*/
        Product* c = v;
        out << c->getid() << ", " << c->getName() << ", ";
        out << c->getCompany() << ", ";
        out << c->getPrice() << "\n";
    }
    file.close( );
}

/*database에 연동이 되는지 확인하는 함수*/
bool ProductManager::productDataConnection()
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

/*수정하고 싶은 아이템을 클릭하였을 때*/
void ProductManager::on_ProductTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);               /*매개인자 column은 쓰이지 않는다*/
    ui->PIDLineEdit->setText(item->text(0));
    ui->PNameLineEdit->setText(item->text(1));
    ui->PCompanyLineEdit->setText(item->text(2));
    ui->PPriceLineEdit->setText(item->text(3));
    ui->toolBox->setCurrentIndex(0);        /*아이템클릭시 툴박스의 위치를 input으로 설정*/
}

int ProductManager::makeID()
{
    if(productList.size() == 0)
    {
        return 1;
    }
    else
    {
        auto id = productList.lastKey();
        return ++id;
    }
}

/*컨텍스트 메뉴를 마우스 우클릭 시 지정된 위치에서 메뉴호출*/
void ProductManager::showContextItem(const QPoint& pos)
{
    QPoint globalPos = ui->ProductTreeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void ProductManager::removeItem()
{
    QTreeWidgetItem* item = ui->ProductTreeWidget->currentItem();
    if(item != nullptr) {
        productList.remove(item->text(0).toInt());
        ui->ProductTreeWidget->takeTopLevelItem
                (ui->ProductTreeWidget->indexOfTopLevelItem(item));
        ui->ProductTreeWidget->update();
    }
    if (!productDataConnection( )) return;           /*고객 데이터베이스에 접근하지 못한 경우*/
    QSqlQueryModel queryModel;
    queryModel.setQuery(QString("CALL DELETE_GOOD(%1)").arg(item->text(0).toInt()));     /*데이터 베이스에서 삭제할 상품 정보*/
}


/*상품의 정보 입력*/
void ProductManager::on_InputButton_clicked()
{
    QString name, company;
    int price;
    int id = makeID( );
    name = ui->PNameLineEdit->text();
    company = ui->PCompanyLineEdit->text();
    price = ui->PPriceLineEdit->text().toInt();
    if(name.length()) {
        Product* p = new Product(id, name, company, price);
        productList.insert(id, p);
        ui->ProductTreeWidget->addTopLevelItem(p);
        emit ProductAdded(name);
    }

    /*상품 데이터베이스가 연결되지 않았을 경우*/
    if (!productDataConnection( )) return;

    QSqlQueryModel queryModel;
    queryModel.setQuery(QString("CALL INSERT_GOOD(%1, '%2', '%3', %4)").arg(id).arg(name).arg(company).arg(price));     /*상품 정보로 들어갈 데이터*/
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
void ProductManager::on_ModifyButton_clicked()
{
    QTreeWidgetItem* item = ui->ProductTreeWidget->currentItem();
    if(item != nullptr) {
        int key = item->text(0).toInt();
        Product* p = productList[key];
        QString name, company;
        int price;
        name = ui->PNameLineEdit->text();
        company = ui->PCompanyLineEdit->text();
        price = ui->PPriceLineEdit->text().toInt();
        p->setName(name);
        p->setCompany(company);
        p->setPrice(price);
        productList[key] = p;

        if (!productDataConnection( )) return;           /*고객 데이터베이스에 접근하지 못한 경우*/
        QSqlQueryModel queryModel;
        queryModel.setQuery(QString("CALL UPDATE_GOOD(%1, '%2', '%3', %4)")
                            .arg(key).arg(name).arg(company).arg(price));     /*데이터 베이스에서 수정할 고객 정보*/
    }
}

/*검색 버튼 클릭시 해당되는 컬럼과 이름의 정보를 출력*/
void ProductManager::on_Search_clicked()
{
    ui->SearchTreeWidget->clear();
    int i = ui->SearchComboBox->currentIndex();
    auto flag = (i)? Qt::MatchCaseSensitive|Qt::MatchContains
                   : Qt::MatchCaseSensitive;
    {
        auto items = ui->ProductTreeWidget->findItems
                (ui->SearchLineEdit->text(), flag, i);

        foreach(auto i, items) {
            Product* p = static_cast<Product*>(i);
            int id = p->getid();
            QString name = p->getName();
            QString company = p->getCompany();
            int price = p->getPrice();
            Product* item = new Product(id, name, company, price);
            ui->SearchTreeWidget->addTopLevelItem(item);
        }
    }
}

/*검색 트리 리스트에서 상품의 이름과 가격을 표시하기 위한 슬롯*/
void ProductManager::on_SearchTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->PNameLineEdit->setText(item->text(1));
    ui->PPriceLineEdit->setText(item->text(3));
}

