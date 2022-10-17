#include "tcplog.h"
#include "ui_tcplog.h"

tcplog::tcplog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tcplog)
{
    ui->setupUi(this);
}

tcplog::~tcplog()
{
    delete ui;
}
