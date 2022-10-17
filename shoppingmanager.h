#ifndef SHOPPINGMANAGER_H
#define SHOPPINGMANAGER_H

#include <QWidget>

class Shopping;
class QTreeWidgetItem;

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
    void on_ShoppingTreeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void showContextMenu(const QPoint&);
    void removeItem();
    void CreceiveData(QString str);
    void PreceiveData(QString str);
    void PreceivePrice(QString price);
    void on_InputButton_clicked();



private slots:
    void on_CancelButton_clicked();

    void on_ModifyButton_clicked();

    void on_SearchButton_clicked();

    void on_SDateLineEdit_returnPressed();

private:
    Ui::ShoppingManager *ui;
    QMap<int, Shopping*>shoppingList;
    QMenu* menu;
    int makeId();
};

#endif // SHOPPINGMANAGER_H
