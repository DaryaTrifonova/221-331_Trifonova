#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newpindialog.h"

#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QTableWidgetItem>

static QString maskedPassword(const QString &real)
{
    const int n = real.isEmpty() ? 3 : real.size();
    return QString(n, QChar(0x25CF));
}

MainWindow::MainWindow(const QVector<Cred>& creds, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , creds_(creds)
{
    ui->setupUi(this);
    connect(ui->credsTable, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onCredDoubleClicked);

    ui->credsTable->setColumnCount(3);
    ui->credsTable->setHorizontalHeaderLabels({"URL", "Логин", "Пароль"});
    ui->credsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->credsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    fillTable();

    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, [this](const QString& text){
        const QString q = text.toLower().trimmed();
        for (int r = 0; r < ui->credsTable->rowCount(); ++r) {
            auto* it = ui->credsTable->item(r, 0);
            const QString url = it ? it->text().toLower() : QString();
            ui->credsTable->setRowHidden(r, !q.isEmpty() && !url.contains(q));
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::loadCredsFromJsonFile(const QString &path)
{
    creds_.clear();

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Ошибка",
                              "Не удалось открыть creds.json.\n"
                              "Положи creds.json рядом с .exe/.bin.\n\nПуть:\n" + path);
        return false;
    }

    const QByteArray data = f.readAll();
    f.close();

    if (data.size() < 2048) {
        QMessageBox::warning(this, "Предупреждение",
                             "creds.json меньше 2 КБ (нужно ≥ 2048 байт).\n"
                             "Текущий размер: " + QString::number(data.size()) + " байт.");
    }

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        QMessageBox::critical(this, "Ошибка",
                              "creds.json не является JSON-массивом.\nОшибка: " + err.errorString());
        return false;
    }

    const QJsonArray arr = doc.array();
    for (const QJsonValue &v : arr) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();

        Cred c;
        c.url = o.value("url").toString();
        c.login = o.value("login").toString();
        c.password = o.value("password").toString();

        if (!c.url.isEmpty())
            creds_.push_back(c);
    }

    if (creds_.size() < 10) {
        QMessageBox::warning(this, "Предупреждение",
                             "В creds.json меньше 10 учётных данных (нужно ≥ 10).\n"
                             "Сейчас: " + QString::number(creds_.size()));
    }

    return true;
}

static QString maskByLen(const QString& s)
{
    const int n = s.isEmpty() ? 3 : s.size();
    return QString(n, QChar(0x25CF));
}

void MainWindow::fillTable()
{
    ui->credsTable->setRowCount(creds_.size());

    for (int row = 0; row < creds_.size(); ++row) {
        const auto& c = creds_[row];

        auto* urlItem = new QTableWidgetItem(c.url);

        auto* loginItem = new QTableWidgetItem(maskByLen(c.login));
        loginItem->setData(Qt::UserRole, c.login);

        auto* passItem = new QTableWidgetItem(maskByLen(c.password));
        passItem->setData(Qt::UserRole, c.password);

        ui->credsTable->setItem(row, 0, urlItem);
        ui->credsTable->setItem(row, 1, loginItem);
        ui->credsTable->setItem(row, 2, passItem);
    }
}

void MainWindow::applyFilter(const QString &query)
{
    const QString q = query.trimmed().toLower();

    for (int row = 0; row < ui->credsTable->rowCount(); ++row) {
        auto *item = ui->credsTable->item(row, 0); // URL
        const QString url = item ? item->text().toLower() : QString();

        const bool match = q.isEmpty() || url.contains(q);
        ui->credsTable->setRowHidden(row, !match);
    }
}

void MainWindow::onCredDoubleClicked(int row, int /*column*/)
{
    auto *urlItem = ui->credsTable->item(row, 0);
    auto *loginItem = ui->credsTable->item(row, 1);
    auto *passItem = ui->credsTable->item(row, 2);

    if (!urlItem || !loginItem || !passItem)
        return;

    const QString url = urlItem->text();
    const QString realPassword = passItem->data(Qt::UserRole).toString();
    const QString login = loginItem->data(Qt::UserRole).toString();

    NewPinDialog dlg;
    if (dlg.exec() == QDialog::Accepted){
        QMessageBox::information(
            this,
            "Учётные данные",
            "Сайт: " + url + "\n" +
                "Логин: " + login + "\n" +
                "Пароль: " + realPassword
            );
    }


}
