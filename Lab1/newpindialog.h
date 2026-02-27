#ifndef NEWPINDIALOG_H
#define NEWPINDIALOG_H

#include <QDialog>
#include <QObject>
#include <QVector>
#include "crypto_utils.h"

namespace Ui {
class NewPinDialog;
}

class NewPinDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewPinDialog(QWidget *parent = nullptr);
    ~NewPinDialog();
    const QVector<Cred>& creds() const { return creds_; }

private slots:
    void on_unlockButton_clicked();

private:
    Ui::NewPinDialog *ui;
    QVector<Cred> creds_;
};

#endif // NEWPINDIALOG_H
