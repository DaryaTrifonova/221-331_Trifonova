#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newpindialog.h"
#include "credviewdialog.h"

#include <QMessageBox>
#include <QTableWidgetItem>

static QString fixedMask()
{
    return QString(8, QChar(0x25CF));
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

    connect(ui->searchLineEdit, &QLineEdit::textChanged,
            this, &MainWindow::applyFilter);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fillTable()
{
    ui->credsTable->setRowCount(creds_.size());

    for (int row = 0; row < creds_.size(); ++row) {
        const auto& c = creds_[row];

        auto* urlItem = new QTableWidgetItem(c.url);
        auto* loginItem = new QTableWidgetItem(fixedMask());
        auto* passItem = new QTableWidgetItem(fixedMask());

        ui->credsTable->setItem(row, 0, urlItem);
        ui->credsTable->setItem(row, 1, loginItem);
        ui->credsTable->setItem(row, 2, passItem);
    }
}

void MainWindow::applyFilter(const QString &query)
{
    const QString q = query.trimmed().toLower();

    for (int row = 0; row < ui->credsTable->rowCount(); ++row) {
        auto *item = ui->credsTable->item(row, 0);
        const QString url = item ? item->text().toLower() : QString();

        const bool match = q.isEmpty() || url.contains(q);
        ui->credsTable->setRowHidden(row, !match);
    }
}

void MainWindow::onCredDoubleClicked(int row, int /*column*/)
{
    if (row < 0 || row >= creds_.size()) {
        return;
    }

    const Cred& selectedCred = creds_[row];

    NewPinDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    QString pin = dlg.acceptedPin();
    QString login;
    QString password;
    QString err;

    if (!decryptSecretFromBase64(selectedCred.secretB64, pin, login, password, err)) {
        secureClearQString(pin);
        QMessageBox::warning(this, "Ошибка", "Не удалось расшифровать выбранную запись.");
        return;
    }

    secureClearQString(pin);

    CredViewDialog credDlg(selectedCred.url, login, password, this);
    credDlg.exec();

    secureClearQString(login);
    secureClearQString(password);
}
