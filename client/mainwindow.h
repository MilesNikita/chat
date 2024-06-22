#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <QDebug>
#include <QMimeDatabase>
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonDocument>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

    void Server_Read_Data();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    QString JsonToString (QJsonObject *a);

    QJsonObject StringToJson (QString a);
    QByteArray encryptXOR(const QByteArray &plainText);

private:
    Ui::MainWindow *ui;
    QTcpSocket* serverConnection = nullptr;
    QTextStream tcpStream;
    QString nickname;
};
#endif // MAINWINDOW_H
