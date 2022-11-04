#include "tcpclient.h"

#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDataStream>
#include <QTcpSocket>
#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QtNetwork>
#include <QPixmap>
#include <QLabel>
#include <QProgressDialog>

#define BLOCK_SIZE      1024

TCPClient::TCPClient(QWidget *parent) : QWidget(parent), isSent(false) {
    // 연결한 서버 정보 입력을 위한 위젯들
    name = new QLineEdit(this);

    /*ipAddress 자동 할당 코드*/
    QString ipAddress;
    QNetworkInterface interface;
    QList<QHostAddress> ipList = interface.allAddresses();
    for (int i = 0; i < ipList.size(); i++)
    {
        if (ipList.at(i) != QHostAddress::LocalHost && ipList.at(i).toIPv4Address())
        {
            ipAddress = ipList.at(i).toString();
            break;
        }
    }
    /*ipAddress 자동 할당 코드*/
    serverAddress = new QLineEdit(this);
    serverAddress->setText(ipAddress);
    //serverAddress->setInputMask("999.999.999.999;_");
    QRegularExpression re("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegularExpressionValidator validator(re);
    serverAddress->setPlaceholderText("Server IP Address"); //IP가 작성된게 없을 경우 출력
    serverAddress->setValidator(&validator);     /*ip주소의 범위 지정(0.0.0.0~255.255.255.255)*/

    serverPort = new QLineEdit(this);
    serverPort->setText(QString::number(PORT_NUMBER));  //현재 설정된 포트 번호 8000
    serverPort->setInputMask("00000;_");                //포트 마스크 5자리만 입력
    serverPort->setPlaceholderText("Server Port No");   //포트가 작성된게 없을 경우 출력

    connectButton = new QPushButton(tr("Log In"), this);
    QHBoxLayout *serverLayout = new QHBoxLayout;
    serverLayout->addWidget(name);          /*접속자의 이름*/
    serverLayout->addStretch(1);            /*공간 스프링*/
    serverLayout->addWidget(serverAddress); /*IP주소*/
    serverLayout->addWidget(serverPort);    /*서버포트*/
    serverLayout->addWidget(connectButton); /*로그인 버튼*/

    message = new QTextEdit(this);		// 서버에서 오는 메시지 표시용
    message->setReadOnly(true);         // 채팅 텍스트 창은 오로지 읽기만 수행

    fileText = new QTextEdit(this);     // 파일 형태로 읽을 수 있는 텍스트 에디터 생성
    fileText->setReadOnly(true);        // 채팅 텍스트와 마찬가지로 읽기만 수행

    IbView = new QLabel(this);          // 이미지를 나타낼 레이블을 생성자에서 초기화

    // 서버로 보낼 메시지를 위한 위젯들
    inputLine = new QLineEdit(this);
    connect(inputLine, SIGNAL(returnPressed( )), SLOT(sendData( )));    /*입력 위젯에 엔터 입력시 문자 전송*/
    connect(inputLine, SIGNAL(returnPressed( )), inputLine, SLOT(clear( ))); /*엔터 입력시 해당 에디터는 지워짐*/
    sentButton = new QPushButton("Send", this);
    connect(sentButton, SIGNAL(clicked( )), SLOT(sendData( ))); /*버튼 클릭시 문자 전송*/
    connect(sentButton, SIGNAL(clicked( )), inputLine, SLOT(clear( ))); /*마찬가지로 에디터의 내용은 지워짐*/
    inputLine->setEnabled(false);       /*접속이 되지 않았으므로 입력에디트는 disable 상태*/
    sentButton->setEnabled(false);      /*버튼도 마찬가지*/

    QHBoxLayout *inputLayout = new QHBoxLayout; /*입력란과 버튼 행정렬*/
    inputLayout->addWidget(inputLine);
    inputLayout->addWidget(sentButton);

    //fileButton = new QPushButton("File Transfer", this);
    //connect(fileButton, SIGNAL(clicked( )), SLOT(sendFile( ))); /*파일 버튼을 누를시 파일을 보내는 커넥트 함수*/
    //fileButton->setDisabled(true);  /*현재 파일 버튼은 disable 상태*/

    findFileButton = new QPushButton("File Find", this);
    connect(findFileButton, SIGNAL(clicked()), SLOT(filereceive()));
    findFileButton->setDisabled(true);

    imageButton = new QPushButton("image Find", this);          /*이미지 버튼 생성*/
    connect(imageButton, &QPushButton::clicked,                 /*람다 함수로 이미지 버튼 클릭시 이벤트 발생*/
            [=]{
        /*현재 이미지르 받은 슬옷은 파일 경로를 받고 클라이언트 채팅창에 이미지로 출력하는 기능을 구현*/
        QString filename = QFileDialog::getOpenFileName(this, "file select",
       /*현 경로는 개발자의 디바이스에서만 연결 가능하오니 다른 디바이스를 사용하는 경우에는 경로를 필히 바꿔주기 바람*/
            "C:\\QtHardWork\\samQtProject-master\\build-Miniproject-Desktop_Qt_6_3_1_MSVC2019_64bit-Debug",
            "image file(*.png *.jpg)");                         /*jpg, png를 부르는 경로 작성*/
        QImage* Img = new QImage();                         /*이미지 변수 생성*/
        QPixmap* buffer = new QPixmap();                    /*픽스맵 변수 생성*/

        qDebug() << filename;

        if(Img->load(filename))                             /*해당경로로 포함된 이미지 파일 호출이 가능하면*/
        {
            *buffer = QPixmap::fromImage(*Img);             /*픽스맵에 Img변수에 이미지 할당*/
            *buffer = buffer->scaled(200, 200, Qt::KeepAspectRatio);  /*사이즈의 크기를 200 by 200으로 채움*/
        }
        else
        {
            QMessageBox::about(0, QString("Image load Error"),      /*만일 이미지를 못 찾을시 메세지박스 호출*/
                               QString("Image load Error"));
        }


        IbView->setPixmap(*buffer);                     /*레이블에 픽스맵 세팅*/
        IbView->resize(200, 200);                       /*레이블의 사이즈도 200 by 200으로 설정*/
        IbView->move(100, 0);                           /*레이블을 가로로 100만큼 움직임*/
        IbView->show();                                 /*레이블 출력*/
    });   /*버튼을 클릭시 이미지를 찾는 버튼을 클릭한 시그널*/
    imageButton->setDisabled(true);

    // 종료 기능
    QPushButton* quitButton = new QPushButton("Log Out", this);
    connect(quitButton, SIGNAL(clicked( )), this, SLOT(close( )));  /*종료 버튼 클릭시 해당 클라이언트는 창이 닫아짐*/

    QHBoxLayout *buttonLayout = new QHBoxLayout;    /*파일 버튼과 종료버튼 행정렬*/
    //buttonLayout->addWidget(fileButton);
    buttonLayout->addWidget(findFileButton);
    buttonLayout->addWidget(imageButton);
    buttonLayout->addStretch(1);                /*공간 스프링*/
    buttonLayout->addWidget(quitButton);

    QHBoxLayout *textlayout = new QHBoxLayout;      /*두 개의 텍스트에디터를 레이아웃*/
    textlayout->addWidget(message);
    textlayout->addWidget(fileText);

    QVBoxLayout *imagelayout = new QVBoxLayout;     /*메세지와 로그를 레이아웃 한 변수와 레이블을 더하는 코드*/
    imagelayout->addLayout(textlayout);
    imagelayout->addWidget(IbView);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);/*행정렬했던 레이아웃과 메세지 창을 모두 수직정렬*/
    mainLayout->addLayout(serverLayout);
    mainLayout->addLayout(imagelayout);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    clientSocket = new QTcpSocket(this);			// 클라이언트 소켓 생성
    connect(clientSocket, &QAbstractSocket::errorOccurred,
            [=]{ qDebug( ) << clientSocket->errorString( ); });
    connect(clientSocket, SIGNAL(readyRead( )), SLOT(receiveData( )));
    connect(clientSocket, SIGNAL(disconnected( )), SLOT(disconnect( )));

    QSettings settings("ChatClient", "Chat Client");
    name->setText(settings.value("ChatClient/ID").toString()); //클라이언트 이름을 적은 후 창을 닫으면 자동으로 저장

    fileClient = new QTcpSocket(this);
    connect(fileClient, SIGNAL(bytesWritten(qint64)), SLOT(goOnSend(qint64)));

    progressDialog = new QProgressDialog(0);        /*프로그래스 다이얼로그 할당*/
    progressDialog->setAutoClose(true);             /*프로그래스 다이얼로그 자동 닫기*/
    progressDialog->reset();                        /*프로그래스 다이얼로그 리셋*/

    connect(connectButton, &QPushButton::clicked,    /*로그인 버튼을 누를시 다른 버튼들과 입력란의 상태가 변동 */
            [=]{
        if(connectButton->text() == tr("Log In")) {
                         /*커넥트 버튼 누를 시 발송되는 시그널*/
            qDebug() << name->text().toStdString().data();
            clientSocket->connectToHost(serverAddress->text( ),
                                        serverPort->text( ).toInt( ));  /*소켓 호스트 연결 (아이피 주소와 포트번호)*/
            clientSocket->waitForConnected();       /*연결을 하기전 잠시 대기하다 연결(충돌방지)*/
            emit compareName(name->text());
            if(result == 0)
            {
                QMessageBox::critical(this, tr("Chatting Client"), \
                                      tr("you got a wrong name"));          /*메세지 박스 춮력*/
                name->clear();                              /*이름 지우기*/

                return;
            }
            sendProtocol(Client_Chat_Login, name->text().toStdString().data()); /*로그인 타입으로 전환*/
            connectButton->setText(tr("Chat in"));                  /*채팅이 가능한 상태의 버튼 이름 변경*/
            name->setReadOnly(true);                                /*성함에디터의 이름이 수정되지 않도록 항상 읽기로 표시*/
        } else if(connectButton->text() == tr("Chat in"))  {        /*버튼의 텍스트가 Chat in 상태라면*/
            sendProtocol(Client_Chat_In, name->text().toStdString().data());    /*채팅중 타입으로 전환*/
            connectButton->setText(tr("Chat Out"));                 /*채팅을 나갈 수 있는 상태의 버튼 이름 변경*/
            inputLine->setEnabled(true);
            sentButton->setEnabled(true);
            //fileButton->setEnabled(true);                   /*버튼 및 발송 에디터의 상태 활성화*/
            findFileButton->setEnabled(true);
            imageButton->setEnabled(true);
        } else if(connectButton->text() == tr("Chat Out"))  {
            sendProtocol(Client_Chat_Out, name->text().toStdString().data());  /*대기실 상태로 전환*/
            connectButton->setText(tr("Chat in"));                  /*채팅에 다시 들어갈 수 있는 버튼 이름 변경*/
            inputLine->setDisabled(true);
            sentButton->setDisabled(true);
            findFileButton->setEnabled(false);
            imageButton->setEnabled(false);
            //fileButton->setDisabled(true);                   /*버튼 및 발송 에디터의 상태 비활성화*/
        }
    } );

    setWindowTitle(tr("Chat Client"));          /*고객용 채팅 방 윈도우 타이틀*/

    this->resize(500, 400);
}

TCPClient::~TCPClient( )
{
    clientSocket->close( );
    QSettings settings("ChatClient", "Chat Client");        /*마지막으로 적었던 이름이*/
    settings.setValue("ChatClient/ID", name->text());       /*프로그램이 종료되어도 clear 되지 않음*/
}

/*현재 파일을 받는 슬롯은 아직 파일의 이름과 형태만 다이얼로그에서 가져다 텍스트 에디트에서 텍스트로 붙임*/
void TCPClient::filereceive()
{
    QString filename = QFileDialog::getOpenFileName(this, "file select",
        "C:\\QtHardWork\\samQtProject-master\\build-Miniproject-Desktop_Qt_6_3_1_MSVC2019_64bit-Debug",
        "Text file(*.text *.txt)");
    qDebug() << filename;               /*콘솔에서만 파일을 출력하게끔 만들고 다른 방법이 있는지 확인*/

    QFileInfo fileInfo(filename);
    if(fileInfo.isReadable())               /*읽을 수 있는 파일인지 확인*/
    {
        QFile* file = new QFile(filename);
        file->open(QIODevice::ReadOnly);        /*파일을 읽기 형태로 불러드림*/
        QByteArray msg = file->readAll();       /*텍스트 파일 전체를 읽어 드림*/
        file->close();                          /*정상적으로 파일을 닫았을 시 0*/
                                                /*그렇지 않을 시 EOF(-1)을 반환*/
        fileText->wordWrapMode();

        delete file;
        fileText->setHtml(msg);

    }
    else
    {
        QMessageBox::warning(this, "Error", "Can't Read this file",
                             QMessageBox::Ok);
    }
}

void TCPClient::closeEvent(QCloseEvent*)
{
    sendProtocol(Client_Chat_LogOut, name->text().toStdString().data());    /*채팅 로그아웃 타입으로 전환*/
    clientSocket->disconnectFromHost();                                 /*현 소켓을 연결 차단*/
    if(clientSocket->state() != QAbstractSocket::UnconnectedState)
        clientSocket->waitForDisconnected();                /*차단시 대기하였다가 차단(충돌방지)*/
}

void TCPClient::receiveData( )      /*또 다른 채팅 클라이언트로 부터 채팅 데이터를 받는 함수*/
{
    QTcpSocket *clientSocket = dynamic_cast<QTcpSocket *>(sender( ));
    if (clientSocket->bytesAvailable( ) > BLOCK_SIZE) return;  //읽기를 기다리고 있는 들어오는 바이트 수를 해당 블록 사이즈 만큼 반환.
    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);

    Client_Chat type;       // 채팅의 목적

    QString ClientName;

    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in.device()->seek(0);

    char data[1020 /*- sizeof(ClientName)*/];        // 전송되는 메시지/데이터
    memset(data, 0, 1020 /*- sizeof(ClientName)*/);
    in >> type /*>> ClientName*/;
    in.readRawData(data, 1020 /*- sizeof(ClientName)*/);

    switch(type) {
    case Client_Chat_Talk:

        //qDebug("[%s] %s : %d", __FILE__, __FUNCTION__, __LINE__);
        if(TCPClient::flag == 0)    /*플래그를 설정해서 0인경우 채팅 활성화*/
        {
            message->append(QString(data));
            inputLine->setEnabled(true);
            sentButton->setEnabled(true);
            //fileButton->setEnabled(true);
            connectButton->setEnabled(true);
            findFileButton->setEnabled(true);
            imageButton->setEnabled(true);
        }
        else    /*한번 강퇴 되면 플래그가 1로 변경되어서 입력문에 채팅을 할 수 없게 됨*/
        {
            inputLine->setDisabled(true);
            inputLine->setEnabled(false);
            sentButton->setEnabled(false);
            //fileButton->setEnabled(false);
            connectButton->setEnabled(false);
            findFileButton->setEnabled(false);
            imageButton->setEnabled(false);
        }
        break;
    case Client_Chat_KickOut:           /*채팅방에서 강제 퇴장 당했을 경우*/
        flag = 1;               /*flag를 1로 설정*/
        QMessageBox::critical(this, tr("Chatting Client"), \
                              tr("Kick out from Server"));
        inputLine->setDisabled(true);
        sentButton->setDisabled(true);
        //fileButton->setDisabled(true);
        findFileButton->setEnabled(false);
        imageButton->setEnabled(false);
        connectButton->setText("Chat in");
        connectButton->setEnabled(false);
        name->setReadOnly(true);
        break;
    case Client_Chat_Invite:            /*채팅방에서 다시 채팅에 초대 되었을 경우*/
        flag = 0;                   /*flag를 0으로 설정*/
        QMessageBox::information(this, tr("Chatting Client"), \
                                 tr("Invited from Server"));
        /*프로토콜을 보내서 챗인 상태로 전환하는 좋은 방법*/
        inputLine->setEnabled(true);
        sentButton->setEnabled(true);
        //fileButton->setEnabled(true);
        findFileButton->setEnabled(true);
        imageButton->setEnabled(true);
        connectButton->setText("Chat Out");
        connectButton->setEnabled(true);
        name->setReadOnly(true);

        break;
    }; /*플래그는 해당되는 채팅클라이언트에서만 상태가 변경*/
}

void TCPClient::disconnect( )
{
    QMessageBox::critical(this, tr("Chatting Client"), \
                          tr("Disconnect from Server"));
    inputLine->setEnabled(false);
    name->setReadOnly(false);
    sentButton->setEnabled(false);
    findFileButton->setEnabled(false);
    imageButton->setEnabled(false);
    connectButton->setText(tr("Log in"));
}

void TCPClient::sendProtocol(Client_Chat type, char* data, int size)
{

    QByteArray dataArray;           // 소켓으로 보낼 데이터를 채우고
    QDataStream out(&dataArray, QIODevice::WriteOnly);
    /*현재 설정된 I/O 장치를 반환하거나 현재 설정된 장치가 없으면 nullptr을 반환합니다.*/
    out.device()->seek(0);          //버퍼를 맨앞에다가 위치
    /*QIODevice를 서브클래싱할 때 QIODevice의 내장 버퍼와의
         * 무결성을 보장하기 위해 함수 시작 시 QIODevice::seek()를 호출해야 합니다.*/
    out << type;
    out.writeRawData(data, size);
    clientSocket->write(dataArray);     // 서버로 전송
    clientSocket->flush();
    while(clientSocket->waitForBytesWritten());
}

void TCPClient::sendData(  )            /*데이터를 보내는 함수*/
{
    QString str = inputLine->text( );
    if(str.length( )) {
        QByteArray bytearray;
        bytearray = str.toUtf8( );      /*Utf8형태로 데이터 전환*/
        message->append("<font color=red>나</font> : " + str); //클라이언ㅌ트
        sendProtocol(Client_Chat_Talk, bytearray.data());
    }
}

void TCPClient::goOnSend(qint64 numBytes) // 파일 내용 보내기 시작
{
    byteToWrite -= numBytes; // 남는 데이터 사이즈
    outBlock = file->read(qMin(byteToWrite, numBytes));
    fileClient->write(outBlock);

    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(totalSize-byteToWrite);

    if (byteToWrite == 0) { // 파일을 모두 전송했을 시
        qDebug("File sending completed!");
        progressDialog->reset();
    }
}

void TCPClient::sendFile() // 파일을 열고 파일 이름(경로 포함)을 가져옵니다.
{
    loadSize = 0;
    byteToWrite = 0;
    totalSize = 0;
    outBlock.clear();           /*파일전송을 위한 가변수 초기화*/

    QString filename = QFileDialog::getOpenFileName(this);
    file = new QFile(filename);
    file->open(QFile::ReadOnly);

    qDebug() << QString("file %1 is opened").arg(filename);
    progressDialog->setValue(0); // 처음으로 전송되지 않음

    if (!isSent) { //처음 전송될 때만 연결이 연결 신호를 생성할 때 발생합니다.

        fileClient->connectToHost(serverAddress->text( ),
                                  serverPort->text( ).toInt( ) + 1);
        isSent = true;
    }

    // 처음으로 보낼 때 connectToHost는 send를 호출하기 위해 연결 신호를 시작하고 두 번째 후에는 send를 호출해야 합니다.

    byteToWrite = totalSize = file->size(); // 나머지 데이터의 크기
    loadSize = 1024; // 매번 전송되는 데이터의 크기

    QDataStream out(&outBlock, QIODevice::WriteOnly);
    out << qint64(0) << qint64(0) << filename;

    totalSize += outBlock.size(); // 전체 크기는 파일 크기에 파일 이름 및 기타 정보의 크기를 더한 것입니다.

    byteToWrite += outBlock.size();
    /*데이터 스트림의 커서를 앞에 위치 시킴*/
    out.device()->seek(0); //바이트 스트림의 시작 부분으로 돌아가 전체 크기와 파일 이름 및 기타 정보 크기인 qint64를 앞에 씁니다.
    out << totalSize << qint64(outBlock.size());

    fileClient->write(outBlock); // 읽은 파일을 소켓으로 보냅니다.

    /*파일 전송의 진행과정을 보여주는 코드*/
    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(totalSize-byteToWrite);
    progressDialog->show();

    qDebug() << QString("Sending file %1").arg(filename);
}

void TCPClient::receiveSignal(int num)
{
    result = num;           /*보내는 시그널의 정수를 받음*/
}
