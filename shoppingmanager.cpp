#include "shoppingmanager.h"
#include "ui_shoppingmanager.h"
#include "shopping.h"

#include <QList>
#include <QStringList>
#include <QString>
#include <QFile>
#include <QMenu>

#include <QDateTimeEdit>
#include <QDateTime>


ShoppingManager::ShoppingManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShoppingManager)
{
    ui->setupUi(this);
    ui->toolBox->setCurrentIndex(0);

    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    menu->addAction(removeAction);
    ui->ShoppingTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->ShoppingTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    /*파일 불러오기*/
    QFile file("shoppinglist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");
        if(row.size()) {
            int id = row[0].toInt();
            int quan = row[4].toInt();
            int price = row[5].toInt();
            Shopping* c = new Shopping(id, row[1], row[2], row[3], quan, price);
            ui->ShoppingTreeWidget->addTopLevelItem(c);
            shoppingList.insert(id, c);
        }
    }
    file.close( );

    ui->SDateLineEdit->setPlaceholderText("press yyyy-mm-dd or Enter");

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
    for (const auto& v : shoppingList) {
        Shopping* s = v;
        out << s->getId() << ", " << s->getClientName() << ", ";
        out << s->getProductName() << ", ";
        out << s->getDate() << ", ";
        out << s->getquan() << ", ";
        out << s->getAllPrice() << "\n";
    }
    file.close( );
}

void ShoppingManager::CreceiveData(QString str)
{
    ui->ClientLineEdit->setText(str);
}

void ShoppingManager::PreceiveData(QString str)
{

    ui->ProductLineEdit->setText(str);
}

void ShoppingManager::PreceivePrice(QString price)
{
    ui->SAllPriceLineEdit->setText(price);
}

int ShoppingManager::makeId( )
{
    if(shoppingList.size( ) == 0) {
        return 1;
    } else {
        auto id = shoppingList.lastKey();
        return ++id;
    }
}

void ShoppingManager::showContextMenu(const QPoint& pos)
{
    QPoint globalPos = ui->ShoppingTreeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void ShoppingManager::removeItem()
{
    QTreeWidgetItem* item = ui->ShoppingTreeWidget->currentItem();
    if(item !=  nullptr)
    {
        shoppingList.remove(item->text(0).toInt());
        ui->ShoppingTreeWidget->takeTopLevelItem
                (ui->ShoppingTreeWidget->indexOfTopLevelItem(item));
        ui->ShoppingTreeWidget->update();
    }
}

void ShoppingManager::on_ShoppingTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->SIDLineEdit->setText(item->text(0));
    ui->ClientLineEdit->setText("choose Client Name");
    ui->ProductLineEdit->setText("choose Product Name"); /*setTextItem*/
    ui->SQuanLineEdit->setText(item->text(4));
    ui->SDateLineEdit->setText(item->text(3));
    //ui->SAllPriceLineEdit->setText(item->text(5));
    ui->SAllPriceLineEdit->setText("");
}


void ShoppingManager::on_InputButton_clicked()
{
    QString client, product, date;
    int id = makeId( );
    int quan, allprice = 0;
    client = ui->ClientLineEdit->text();
    product = ui->ProductLineEdit->text();
    date = ui->SDateLineEdit->text();
    quan = ui->SQuanLineEdit->text().toInt();
    allprice = ui->SAllPriceLineEdit->text().toInt() * quan;

    if(client.length() && product.length()) {
        Shopping* s = new Shopping(id, client, product, date, quan, allprice);
        shoppingList.insert(id, s);
        ui->ShoppingTreeWidget->addTopLevelItem(s);
    }
}


void ShoppingManager::on_CancelButton_clicked()
{
    ui->SIDLineEdit->setText("");
    ui->ClientLineEdit->setText("");
    ui->ProductLineEdit->setText("");
    ui->SDateLineEdit->setText("");
    ui->SQuanLineEdit->setText("");
    ui->SAllPriceLineEdit->setText("");
}


void ShoppingManager::on_ModifyButton_clicked()
{
    QTreeWidgetItem* item = ui->ShoppingTreeWidget->currentItem();
    if(item != nullptr) {
        int key = item->text(0).toInt();
        Shopping* s = shoppingList[key];
        QString client, product, date;
        int quan, price;
        client = ui->ClientLineEdit->text();
        product = ui->ProductLineEdit->text();
        date = ui->SDateLineEdit->text();
        quan = ui->SQuanLineEdit->text().toInt();
        price = ui->SAllPriceLineEdit->text().toInt() * quan;

        s->setClientName(client);
        s->setProductName(product);
        s->setDate(date);
        s->setquan(quan);
        s->setAllPrice(price);
        shoppingList[key] = s;
    }
}


void ShoppingManager::on_SearchButton_clicked()
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


void ShoppingManager::on_SDateLineEdit_returnPressed()
{
    QDateTimeEdit* datetimeedit = new QDateTimeEdit(QDate::currentDate(), 0);
    //datetimeedit->setCalendarPopup(true);
    ui->SDateLineEdit->setText(datetimeedit->text());
}

