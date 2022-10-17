#include "productmanager.h"
#include "ui_productmanager.h"
#include "product.h"

#include <QFile>
#include <QMenu>

ProductManager::ProductManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductManager)
{
    ui->setupUi(this);
    ui->toolBox->setCurrentIndex(0);

    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    menu->addAction(removeAction);
    ui->ProductTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->ProductTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextItem(QPoint)));

    /*데이터 보내기*/
    connect(ui->PNameLineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(ProductAdded(QString)));

    connect(ui->PPriceLineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(ProductPrices(QString)));


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
            ui->ProductTreeWidget->addTopLevelItem(c);
            productList.insert(id, c);

            emit ProductAdded(row[1]);
        }
    }
    file.close( );
}

ProductManager::~ProductManager()
{
    delete ui;

    /*파일 저장*/
    QFile file("productList.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (const auto& v : productList) {
        Product* c = v;
        out << c->getid() << ", " << c->getName() << ", ";
        out << c->getCompany() << ", ";
        out << c->getPrice() << "\n";
    }
    file.close( );
}

QString ProductManager::getProductName()
{
    return ui->PNameLineEdit->text();
}

void ProductManager::on_ProductTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->PIDLineEdit->setText(item->text(0));
    ui->PNameLineEdit->setText(item->text(1));
    ui->PCompanyLineEdit->setText(item->text(2));
    ui->PPriceLineEdit->setText(item->text(3));
    ui->toolBox->setCurrentIndex(0);
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

int ProductManager::getProductPrice()
{
    return ui->PPriceLineEdit->text().toInt();
}

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
}

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
}


void ProductManager::on_CancelButton_clicked()
{
    ui->PIDLineEdit->setText("");
    ui->PNameLineEdit->setText("");
    ui->PCompanyLineEdit->setText("");
    ui->PPriceLineEdit->setText("");
}


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
    }
}

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


void ProductManager::on_SearchTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->PNameLineEdit->setText(item->text(1));
    ui->PPriceLineEdit->setText(item->text(3));
}

