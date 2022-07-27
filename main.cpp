#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QString>
#include <QSettings>
#include <QDebug>
#include <QSharedMemory>
#include <QTextCodec>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSharedMemory shared;
            shared.setKey("hefei-knox-master-progress");
            if(shared.isAttached()){
                if(!shared.detach()){
                    qDebug()<<shared.errorString();
                    return -1;
                }
                else{
                    qDebug()<< "shared memory success";
                }
            }
            if(shared.create(1024)){
                qDebug()<< "create shared memory success";
            }
            else{
                qDebug()<< shared.errorString();
                return -1;
            }
    MainWindow w;
    w.show();
    return a.exec();
}
