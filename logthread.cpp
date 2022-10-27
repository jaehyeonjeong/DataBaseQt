#include "logthread.h""

#include <QTreeWidgetItem>
#include <QFile>
#include <QDateTime>
#include <QMessageBox>

LogThread::LogThread(QObject *parent)
    : QThread{parent}
{

}

void LogThread::run()       /*프로그램 실행시 자동으로 동작하는 run()*/
{
    Q_FOREVER {     /*로그가 있을때 기록, 아이템의 갯수를 반환*/
        if(itemList.count() > 0) {      /*강사님께 질문하기 */
            QString format = "yyyyMMdd_hhmmss";     /*년 월 일 _ 시 분 초 포멧*/
            QString filename = QString("log_%1.txt").arg(QDateTime::currentDateTime().toString(format));
            //로그로 저장되는 파일 이름 출력
            QFile file(filename);   /*파일 이름 지정한 형태(날짜/시간)로 작성*/
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) /*파일 저장 과정 open()*/
                return;

            QTextStream out(&file);     /*텍스트 변수 설정*/
            foreach(auto item, itemList) {
                out << item->text(0) << ", ";
                out << item->text(1) << ", ";
                out << item->text(2) << ", ";
                out << item->text(3) << ", ";
                out << item->text(4) << ", ";
                out << item->text(5) << "\n";
            }
            file.close();/*파일 저장 과정 close()*/
        }
        sleep(60);      // Q_FOREVER을 1분마다 파일에 기록
    }
}

/*서버내의 저장버튼 클릭 시 저장할 수 있는 슬롯 추가*/
void LogThread::saveData()      /*서버내의 저장버튼 입력시 시그널을 받을 슬롯 함수 구현*/
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

    QMessageBox::information(nullptr, "save log box", "log save commplete",
                             QMessageBox::NoButton);    /*저장 완료시 메세지 박스 출력*/
}

void LogThread::appendData(QTreeWidgetItem* item)
{
    itemList.append(item);
}
