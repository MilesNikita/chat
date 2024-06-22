#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::AnyIPv4, 5555);
    connect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::ClientConnected);
}

MainWindow::~MainWindow()
{
    for (QTcpSocket* client : clientConnections) {
        if (client) {
            client->disconnect();
            client->deleteLater();
        }
    }
    delete ui;
}

void MainWindow::ClientConnected()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    if (!clientConnection) {
        return;
    }
    connect(clientConnection, &QTcpSocket::disconnected, this, &MainWindow::ClientDisconnected);
    connect(clientConnection, &QTcpSocket::readyRead, this, &MainWindow::ClientReadData);
    clientConnections.append(clientConnection);
    sendUserListToClients();
}

void MainWindow::ClientDisconnected()
{
    QTcpSocket* disconnectedClient = qobject_cast<QTcpSocket*>(sender());
    if (disconnectedClient) {
        clientConnections.removeAll(disconnectedClient);
        disconnectedClient->deleteLater();
    }
    sendUserListToClients();
}

void MainWindow::ClientReadData()
{
    QTcpSocket* senderClient = qobject_cast<QTcpSocket*>(sender());
    if (!senderClient) {
        return;
    }

    QByteArray data = senderClient->readAll();
    QByteArray decryptedData = encryptXOR(data);
    if (decryptedData.isEmpty()) {
        qWarning() << "Ошибка при дешифровании данных.";
        return;
    }
    qDebug() << decryptedData;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(decryptedData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qDebug() << jsonDoc;
        qWarning() << "Ошибка при преобразовании данных JSON.";
        return;
    }

    QJsonObject response = jsonDoc.object();
    if (response.contains("nickname")) {
        QString nickname = response["nickname"].toString();
        QString ip = senderClient->peerAddress().toString();
        userNicknames[ip] = nickname;
        ui->listWidget_2->addItem(nickname);
    }
    if (response.contains("text")) {
        QString nickname = response["nick"].toString();
        ui->listWidget->addItem(nickname + " : " + response["text"].toString());
        for (QTcpSocket* client : clientConnections) {
            if (client) {
                client->write(encryptXOR(decryptedData));
                client->flush();
            }
        }
    }
    if (response.contains("file_name")) {
        QString nickname = response["nick"].toString();
        ui->listWidget->addItem(nickname + " : файл");
        for (QTcpSocket* client : clientConnections) {
            if (client) {
                if (client->isOpen() && client->isWritable()) {
                    client->write((encryptXOR(decryptedData)));
                    client->flush();
                } else {
                    qWarning() << "Не получилось отправить: " << client->peerAddress().toString();
                }
            }
        }
    }
}

QString MainWindow::JsonToString(QJsonObject *a)
{
    return QString(QJsonDocument(*a).toJson(QJsonDocument::Compact));
}

QJsonObject MainWindow::StringToJson(QString a)
{
    return QJsonDocument::fromJson(a.toUtf8()).object();
}

QByteArray MainWindow::encryptXOR(const QByteArray &plainText)
{
    QByteArray key = "[sdfmpofgjdfpgjpgjfdpvndfv]pdfpvinsf[on";
    QByteArray encryptedData;
    int keyLength = key.length();
    for (int i = 0; i < plainText.size(); ++i) {
        encryptedData.append(plainText.at(i) ^ key.at(i % keyLength));
    }
    return encryptedData;
}

void MainWindow::sendUserListToClients()
{
    QStringList userList;
    for (QTcpSocket* client : clientConnections) {
        if (client) {
            QString ip = client->peerAddress().toString();
            if (userNicknames.contains(ip)) {
                userList.append(userNicknames[ip]);
            } else {
                userList.append(ip);
            }
        }
    }
    QJsonObject obj;
    obj["user_list"] = QJsonArray::fromStringList(userList);
    QByteArray data = encryptXOR(JsonToString(&obj).toUtf8());
    for (QTcpSocket* client : clientConnections) {
        if (client) {
            client->write(data);
            client->flush();
        }
    }
}
