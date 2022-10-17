#ifndef PRODUCTMANAGER_H
#define PRODUCTMANAGER_H

#include <QWidget>

class Product;
class QMenu;
class QTreeWidgetItem;

namespace Ui {
class ProductManager;
}

class ProductManager : public QWidget
{
    Q_OBJECT

public:
    explicit ProductManager(QWidget *parent = nullptr);
    ~ProductManager();
    QString getProductName();
    int getProductPrice();

signals:
    void ProductAdded(QString);
    void ProductPrices(QString);

private slots:
    void on_ProductTreeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void showContextItem(const QPoint&);
    void removeItem();
    void on_InputButton_clicked();
    void on_CancelButton_clicked();
    void on_ModifyButton_clicked();
    void on_Search_clicked();


    void on_SearchTreeWidget_itemClicked(QTreeWidgetItem *item, int column);

private:
    Ui::ProductManager *ui;
    int makeID();
    QMap<int, Product*>productList;
    QMenu* menu;
};

#endif // PRODUCTMANAGER_H
