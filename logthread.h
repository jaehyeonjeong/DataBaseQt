#ifndef LOGTHREAD_H
#define LOGTHREAD_H

#include <QList>
#include <QThread>

class QTreeWidgetItem;

class LogThread : public QThread
{
    Q_OBJECT
public:
    explicit LogThread(QObject *parent = nullptr);

signals:
    void send(int data);

public slots:
    void appendData(QTreeWidgetItem*);

private:
    void run();

    QList<QTreeWidgetItem*> itemList;

};

#endif // LOGTHREAD_H
