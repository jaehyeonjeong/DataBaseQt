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
class QStandardItemModel;

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
    void on_tableView_clicked(const QModelIndex &index);
    void on_RecentButton_clicked();
    void on_TBSearchView_clicked(const QModelIndex &index);

private:
    Ui::ProductManager *ui;
    int makeID();
    QMap<int, Product*>productList;
    QMenu* menu;

    QSqlTableModel* ProductModel;
    QSqlQuery* ProductQuery;

    QStandardItemModel* SearchModel;        /*검색을 하기 위한 모델 변수 선언*/
};

#endif // PRODUCTMANAGER_H
