#include "newpindialog.h"
#include "ui_newpindialog.h"

NewPinDialog::NewPinDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewPinDialog)
{
    ui->setupUi(this);
}

NewPinDialog::~NewPinDialog()
{
    delete ui;
}

void NewPinDialog::on_unlockButton_clicked()
{
    const QString pin = ui->pinLineEdit->text();
    if (pin == "1234"){
        accept();
        return;
    }
    ui->errorLabel->setText("Неверный пароль!");
    ui->pinLineEdit->clear();
    ui->pinLineEdit->setFocus();
}

