#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QList>
#include <QThread>


class QTreeWidgetItem;

class clientThread : public QThread //QThread 상속
{
    Q_OBJECT
public:
    explicit clientThread(QObject *parent = nullptr); //생성자 초기화

signals:
    void send(int data);   //응답을 기다리지 않고 이 연결을 통해
    //메시지를 보냅니다. 이는 오류, 신호 및 반환 값과 반환 값이
    //필요하지 않은 호출에 적합합니다.(시스템 함수)

public slots:
    void appendData(QTreeWidgetItem*);
    //저장된 문자열에 QTreeWidgetItem 형태로 추가합니다.

    void saveData();    //save저장 시그널을 받는 슬롯

private:
    void run();  //지정된 <arguments>를 사용하여 지
    //정된 제품의 실행 파일을 실행합니다. (시스템 함수)
    QString filename;

    QList<QTreeWidgetItem*> clientList;
};


#endif // CLIENTTHREAD_H


