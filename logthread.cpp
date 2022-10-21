#include "logthread.h""

#include <QTreeWidgetItem>
#include <QFile>
#include <QDateTime>
#include <QMessageBox>

LogThread::LogThread(QObject *parent)
    : QThread{parent}
{

}

void LogThread::run()
{
    Q_FOREVER {
        if(itemList.count() > 0) {
            QString format = "yyyyMMdd_hhmmss";
            QString filename = QString("log_%1.txt").arg(QDateTime::currentDateTime().toString(format));
            QFile file(filename);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                return;

            QTextStream out(&file);
            foreach(auto item, itemList) {
                out << item->text(0) << ", ";
                out << item->text(1) << ", ";
                out << item->text(2) << ", ";
                out << item->text(3) << ", ";
                out << item->text(4) << ", ";
                out << item->text(5) << "\n";
            }
            file.close();
        }
        sleep(60);      // 1분마다 저장
    }
}

/*서버내의 저장버튼 클릭 시 저장할 수 있는 슬롯 추가*/
void LogThread::saveData()
{
    QFile file("logSavefile.txt");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    foreach(auto item, itemList)
    {
        out << item->text(0) << ", ";
        out << item->text(1) << ", ";
        out << item->text(2) << ", ";
        out << item->text(3) << ", ";
        out << item->text(4) << ", ";
        out << item->text(5) << "\n";
    }
    file.close();

    //QMessageBox::StandardButton QMessageBox::information(QWidget *parent,
    //const QString &title, const QString &text, QMessageBox::StandardButtons buttons = Ok,
    //QMessageBox::StandardButton defaultButton = NoButton)

    QMessageBox::information(nullptr, "save log box", "log save commplete",
                             QMessageBox::NoButton);
}

void LogThread::appendData(QTreeWidgetItem* item)
{
    itemList.append(item);
}
