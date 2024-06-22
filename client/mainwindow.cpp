#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serverConnection = new QTcpSocket(this);
    connect(serverConnection, &QTcpSocket::readyRead, this, &MainWindow::Server_Read_Data);
}

MainWindow::~MainWindow()
{
    serverConnection->abort();
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    if (!nickname.isEmpty()) {
        serverConnection->abort();
        serverConnection->connectToHost(ui->lineEdit->text(), ui->lineEdit_2->text().toInt());
        if (serverConnection->waitForConnected()) {
            ui->pushButton_3->setEnabled(true);
            ui->pushButton->setEnabled(false);
            QJsonObject obj;
            obj["nickname"] = nickname;
            QByteArray data = encryptXOR(JsonToString(&obj).toUtf8());
            serverConnection->write(data);
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к серверу.");
        }
    } else {
        QMessageBox::critical(this, "Ошибка", "Укажите никнейм пользователя");
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    serverConnection->close();
    ui->pushButton->setEnabled(true);
    ui->pushButton_3->setEnabled(false);
}

void MainWindow::on_pushButton_2_clicked()
{
    if (!nickname.isEmpty()) {
        if (!ui->lineEdit_3->text().isEmpty()) {
            QJsonObject obj;
            obj["nick"] = nickname;
            obj["text"] = ui->lineEdit_3->text();
            QByteArray data = encryptXOR(JsonToString(&obj).toUtf8());
            serverConnection->write(data);
        }
    } else {
        QMessageBox::critical(this, "Ошибка", "Укажите никнейм пользователя");
    }
}

void MainWindow::Server_Read_Data()
{
    QByteArray data = encryptXOR(serverConnection->readAll());
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        return;
    }
    qDebug() << data;
    QJsonObject response = jsonDoc.object();
    if (response.contains("text")) {
        QString text = response["text"].toString();
        QString nick = response["nick"].toString();
        ui->listWidget->addItem(nick + " : " + text);
    }
    if (response.contains("file_name")) {
        QString fileName = response["file_name"].toString();
        QString fileType = response["type"].toString();
        QByteArray fileData = QByteArray::fromBase64(response["file_data"].toString().toUtf8());
        QString nick = response["nick"].toString();
        QFile receivedFile(fileName);
        if (receivedFile.open(QIODevice::WriteOnly)) {
            receivedFile.write(fileData);
            receivedFile.close();
        }
        ui->listWidget->addItem(nick + " Получен файл: " + fileName + " типа: " + fileType);
    }
    if (response.contains("user_list")) {
        QJsonArray userList = response["user_list"].toArray();
        QStringList stringList;
        for (const QJsonValue &value : userList) {
            stringList.append(value.toString());
        }
        ui->listWidget_2->clear();
        ui->listWidget_2->addItems(stringList);
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    if (!nickname.isEmpty()) {
        QString filePath = QFileDialog::getOpenFileName(this, "Выбрать файл");
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                QString fileName = QFileInfo(filePath).fileName();
                QMimeDatabase mimeDatabase;
                QString mimeType = mimeDatabase.mimeTypeForFile(filePath).name();
                QByteArray fileData = file.readAll();
                QJsonObject obj;
                obj["file_name"] = fileName;
                obj["type"] = mimeType;
                obj["nick"] = nickname;
                obj["file_data"] = QString(fileData.toBase64());
                QByteArray data = encryptXOR(JsonToString(&obj).toUtf8());
                serverConnection->write(data);
                file.close();
            }
        }
    } else {
        QMessageBox::critical(this, "Ошибка", "Укажите никнейм пользователя");
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    nickname = QInputDialog::getText(this, "Укажите никнейм", "Введите никнейм пользователя");
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
