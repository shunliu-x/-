#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFile>
#include <QImage>
#include <QDebug>
#include <QSplashScreen>
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QPen>
#include <QTime>
#include <QTimer>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    current_show_image_num = 0;
    port = 8200;
    slave_addr="192.168.2.19";
    slave_port = 8100;
    master_image_directory = QDir::currentPath()+"/image/";
    socket = new QUdpSocket;
    socket->bind(QHostAddress("192.168.2.62") , port);
    connect(socket , SIGNAL(readyRead()) , this , SLOT(receive()));
    font.setPixelSize(30);
    get_all_master_image();
    if(open_slave_program()){
        get_all_slave_image();
        syn_slave_image();
        showFullScreen();
        send("show");
        show_image(current_show_image_num);
    }else{
        ui->label->setText("error");
    }
}

MainWindow::~MainWindow()
{
    qDebug()<<"end process and end slave";
    close_slave_program();
    delete socket;
    delete ui;
}

bool MainWindow::open_slave_program(){
    send("open");
    delay_ms(1000);
    if(QString(recv_buf) == "ok"){
        recv_buf.clear();
        return true;
    }

    return false;
}

void MainWindow::close_slave_program(){
    send("close");
}

void MainWindow::get_all_master_image(){

    QDir dir(master_image_directory);
    if(!dir.exists()){
        return ;
    }
    QStringList name_fliters;
    name_fliters<<"*.jpg"<<"*.png"<<"*.bmp";
    QStringList tem = dir.entryList(name_fliters , QDir::Files| QDir::Readable , QDir::Name);
    qDebug()<<"tem isze:"<<tem.size();
    for(QString &s:tem){
        QImage img(master_image_directory+s);
        qDebug()<<"imge:"<<s<<" w:"<<img.width()<<" h:"<<img.height();
        if(img.width()==3840 && img.height() == 2160){
            master_image_name.append(s);
        }
    }

}

void MainWindow::get_all_slave_image(){
    send("syn");
    delay_ms(4000);
    QString str = QString(recv_buf);
    qDebug()<<"slave image num size:"<<recv_buf.size();
    qDebug()<<recv_buf;
    if(str.endsWith("ok")){
        slave_image_name = str.split("\n");
        qDebug()<<"savle img num:"<<slave_image_name.size()-1;
        slave_image_name.pop_back();
    }
}

void MainWindow::syn_slave_image(){
    int cnt1  = master_image_name.size(), cnt2 = slave_image_name.size();
    if(cnt1 == 0 && cnt2 != 0){
        send("clear");
        return ;
    }
    bool need_clear = false;
    QStringList sendimg;
    for(int i = 0 ; i < cnt2 ; ++ i){
        if(master_image_name.contains(slave_image_name[i])){

        }else{
            need_clear = true;
            break;
        }
    }
    if(need_clear){
        send("clear");
        delay_ms(100);
        QString str = "addimage\n";
        for(QString &s:master_image_name){
            str+=s;
            str+="\n";
        }
        str+="ok";
        send(str);
        qDebug()<<"send:"<<str;
        delay_ms(50);
        for(QString& s:master_image_name){
            QFile f(master_image_directory+s);
            f.open(QIODevice::ReadOnly);
            while(!f.atEnd()){
                QByteArray line = f.read(8000);
                socket->writeDatagram(line , QHostAddress(slave_addr),slave_port);
                delay_ms(10);
            }
            send("ok");
            delay_ms(100);
            f.close();
        }
        send("end");
    }else{
        for(int i = 0 ; i < cnt1 ;++ i){
            if(!slave_image_name.contains(master_image_name[i])){
                sendimg.append(master_image_name[i]);
            }else{

            }
        }
        QString str = "addimage\n";
        for(QString &s:sendimg){
            str+=s;
            str+="\n";
        }
        str+="ok";
        send(str);
        qDebug()<<"send:"<<str;
        delay_ms(50);
        for(QString& s:sendimg){
            QFile f(master_image_directory+s);
            f.open(QIODevice::ReadOnly);
            while(!f.atEnd()){
                QByteArray line = f.read(8000);
                socket->writeDatagram(line , QHostAddress(slave_addr),slave_port);
                delay_ms(10);
            }
            send("ok");
            f.close();
            delay_ms(100);
        }
        send("end");
    }
}

void MainWindow::show_image(int num){
    qDebug()<<"img num:"<<num<<" and all size:"<<master_image_name.size()<<" is equal:"<<(num == master_image_name.size());
    if(num == master_image_name.size() || num < 0){

        close();
        return ;
    }

    QImage pixmap(master_image_directory+master_image_name[num]);
    //pixmap  =  pixmap.copy(0,0,1920,2160);
    pixmap  =  pixmap.copy(0,0,1920,2160);
    QScreen *screen = qApp->screens().at(0);
    //QString show_text = QString::number(screen->size().width())+"x"+QString::number(screen->size().height());//+""+QString::number(screen->refreshRate())+"Hz";
    QFontMetrics fm(font);
    QPainter painter(&pixmap);
    QString show_text = "3840x2160";
    int char_height = fm.height();
    //show_text+=QString::number(screen->refreshRate())+"Hz";
    painter.setCompositionMode(QPainter::CompositionMode_SourceOut);
    QPen pen = painter.pen();
    painter.setPen(pen);
    painter.setFont(font);
    painter.drawText(50,2*char_height,show_text);
    painter.drawText(50,3*char_height,QString::number(screen->refreshRate())+"Hz");
    ui->label->setPixmap(QPixmap::fromImage(pixmap));
}

void MainWindow::receive(){
    while(socket->hasPendingDatagrams()){
        recv_buf.resize(socket->pendingDatagramSize());
        socket->readDatagram(recv_buf.data() , recv_buf.size());
    }
}
void MainWindow::send(QString args){
    socket->writeDatagram(args.toLocal8Bit() , QHostAddress(slave_addr) , slave_port);
}
void MainWindow::keyPressEvent(QKeyEvent *event){
    switch (event->key()) {
    case Qt::Key_Right:
        ++current_show_image_num;
        send("right");
        show_image(current_show_image_num);
        break;
    case Qt::Key_Left:
        --current_show_image_num;
        send("left");
        //if(current_show_image_num<0)current_show_image_num+=master_image_name.size();
        show_image(current_show_image_num);
        break;
    case Qt::Key_Escape:
        close();
        break;
    default:
        break;
    }
}

void MainWindow::delay_ms(int ms)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(ms, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}
