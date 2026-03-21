#include "credviewdialog.h"
#include "ui_credviewdialog.h"
#include "crypto_utils.h"

#include <QApplication>
#include <QClipboard>
#include <QTimer>

CredViewDialog::CredViewDialog(const QString& url,
                               const QString& login,
                               const QString& password,
                               QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CredViewDialog)
    , login_(login)
    , password_(password)
{
    ui->setupUi(this);

    ui->urlLineEdit->setText(url);
    ui->loginLineEdit->setText(login_);
    ui->passLineEdit->setText(password_);
    ui->statusLabel->clear();
}

CredViewDialog::~CredViewDialog()
{
    ui->loginLineEdit->clear();
    ui->passLineEdit->clear();

    secureClearQString(login_);
    secureClearQString(password_);

    delete ui;
}

void CredViewDialog::on_copyLoginButton_clicked()
{
    QApplication::clipboard()->setText(login_);
    ui->statusLabel->setText("Логин скопирован успешно");
    QTimer::singleShot(1500, ui->statusLabel, &QLabel::clear);
}

void CredViewDialog::on_copyPasswordButton_clicked()
{
    QApplication::clipboard()->setText(password_);
    ui->statusLabel->setText("Пароль скопирован успешно");
    QTimer::singleShot(1500, ui->statusLabel, &QLabel::clear);
}
