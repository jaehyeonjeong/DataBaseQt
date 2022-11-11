#ifndef SHOPPINGMANAGER_H
#define SHOPPINGMANAGER_H

#include <QWidget>

class Shopping;
class QTreeWidgetItem;
class QSqlTableModel;
class QSqlQuery;
class QStandardItemModel;

namespace Ui {
class ShoppingManager;
}

class ShoppingManager : public QWidget
{
    Q_OBJECT

public:
    explicit ShoppingManager(QWidget *parent = nullptr);
    ~ShoppingManager();


public slots:
    void showContextMenu(const QPoint&);
    void removeItem();
    void CreceiveData(QString str);
    void PreceiveData(QString str);
    void PreceivePrice(QString price);
    void on_InputButton_clicked();
    void on_CancelButton_clicked();
    void on_ModifyButton_clicked();
    void on_SearchButton_clicked();
    void on_tableView_clicked(const QModelIndex &index);
    void on_RecentButton_clicked();

private:
    Ui::ShoppingManager *ui;
    QMap<int, Shopping*>shoppingList;
    QMenu* menu;
    int makeId();

    QSqlTableModel* ShoppingModel;
    QSqlQuery* ShoppingQuery;

    QStandardItemModel* SearchModel;
};

#endif // SHOPPINGMANAGER_H
