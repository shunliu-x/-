#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //open/close slave program
    bool open_slave_program();
    void close_slave_program();

    void get_all_master_image();
    void get_all_slave_image();
    void syn_slave_image();
    void show_image(int num);
    void send(QString args);
    void delay_ms(int ms);
public slots:
    void receive();

private:
    Ui::MainWindow *ui;
    bool slave_is_open;
    QStringList master_image_name;
    QStringList slave_image_name;
    QString master_image_directory;
    int current_show_image_num;
    QFont font;
    QUdpSocket *socket;
    QByteArray recv_buf;
    int port;
    int slave_port;
    QString slave_addr;
protected:
    void keyPressEvent(QKeyEvent *event);
};
#endif // MAINWINDOW_H
