#ifndef TCPLOG_H
#define TCPLOG_H

#include <QWidget>

namespace Ui {
class tcplog;
}

class tcplog : public QWidget
{
    Q_OBJECT

public:
    explicit tcplog(QWidget *parent = nullptr);
    ~tcplog();

private:
    Ui::tcplog *ui;
};

#endif // TCPLOG_H
