#include "clientthread.h"

#include <QFile>
#include <QDateTime>
#include <QMessageBox>
#include <QTreeWidgetItem>

clientThread::clientThread(QObject* parent)
    : QThread{parent}
{
    QString format = "yyyyMMdd_hhmmss";     /*년 월 일 _ 시 분 초 포멧*/
    filename = QString("client_%1.txt").arg(QDateTime::currentDateTime().toString(format));
}

void clientThread::run()       /*프로그램 실행시 자동으로 동작하는 run()*/
{
    Q_FOREVER {     /*로그가 있을때 기록, 아이템의 갯수를 반환*/
        saveData();
        sleep(60);      // Q_FOREVER을 1분마다 파일에 기록
    }
}

/*서버내의 저장버튼 클릭 시 저장할 수 있는 슬롯 추가*/
void clientThread::saveData()      /*서버내의 저장버튼 입력시 시그널을 받을 슬롯 함수 구현*/
{
    if(clientList.count() > 0)
    {
        QFile file(filename);

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream out(&file);
        foreach(auto item, clientList)
        {
            out << item->text(0) << ", ";
            out << item->text(1) << ", ";
            out << item->text(2) << ", ";
            out << item->text(3) << "\n";
            qDebug() << item->text(0) << " " << item->text(1);
        }
        file.close();

    }

}

void clientThread::appendData(QTreeWidgetItem* item)
{
    clientList.append(item);
}
