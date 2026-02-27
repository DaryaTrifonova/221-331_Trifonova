#include "newpindialog.h"
#include "ui_newpindialog.h"
#include <QTimer>
#include <QDir>
#include <QCoreApplication>

NewPinDialog::NewPinDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewPinDialog)
{
    ui->setupUi(this);
    ui->toastframe->hide();
}

NewPinDialog::~NewPinDialog()
{
    delete ui;
}

void NewPinDialog::on_unlockButton_clicked()
{
    const QString pin = ui->pinLineEdit->text();
    const QString vaultPath = QDir(QCoreApplication::applicationDirPath()).filePath("vault.enc");

    QString err;
    QVector<Cred> tmp;

    if (!decryptVaultFromFile(vaultPath, pin, tmp, err)) {
        ui->pinLineEdit->clear();
        ui->pinLineEdit->setFocus();
        ui->toastframe->show();
        QTimer::singleShot(1500, ui->toastframe, &QWidget::hide);
        return;
    }

    creds_ = std::move(tmp);
    accept();
}


