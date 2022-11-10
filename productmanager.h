#ifndef PRODUCTMANAGER_H
#define PRODUCTMANAGER_H

#include "qsqldatabase.h"
#include <QWidget>

class Product;
class QMenu;
class QTreeWidgetItem;
class QSqlQueryModel;
class QSqlRelationalTableModel;
class QSqlQuery;
class QModalIndex;
class QSqlDatabase;
class QSqlTableModel;

namespace Ui {
class ProductManager;
}

class ProductManager : public QWidget
{
    Q_OBJECT

public:
    explicit ProductManager(QWidget *parent = nullptr);     /*ProductManager생성자 초기화*/
    ~ProductManager();

signals:
    void ProductAdded(QString);
    void ProductPrices(QString);

private slots:
    void showContextItem(const QPoint&);
    void removeItem();
    void on_InputButton_clicked();
    void on_CancelButton_clicked();
    void on_ModifyButton_clicked();
    void on_Search_clicked();
    void on_SearchTreeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_tableView_clicked(const QModelIndex &index);

    void on_RecentButton_clicked();

private:
    Ui::ProductManager *ui;
    int makeID();
    QMap<int, Product*>productList;
    QMenu* menu;

    QSqlTableModel* ProductModel;
    QSqlQueryModel* ProductqueryModel;
    QSqlQuery* ProductQuery;
};

#endif // PRODUCTMANAGER_H
