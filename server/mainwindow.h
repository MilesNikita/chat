#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QTextStream>
#include <QMessageBox>
#include <QVector>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QByteArray>
#include <QDebug>
#include <QRandomGenerator>
#include <QTextCodec>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void ClientConnected();
    void ClientDisconnected();
    void ClientReadData();

    QString JsonToString (QJsonObject *a);
    QJsonObject StringToJson (QString a);
    QByteArray encryptXOR(const QByteArray &plainText);
    void sendUserListToClients();

private:
    Ui::MainWindow *ui;
    QTcpServer* tcpServer;
    QVector<QTcpSocket*> clientConnections;
    QTcpSocket* clientConnection;
    QMap<QString, QString> userNicknames;
};
#endif // MAINWINDOW_H
