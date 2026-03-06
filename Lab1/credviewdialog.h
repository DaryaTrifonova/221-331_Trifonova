#ifndef CREDVIEWDIALOG_H
#define CREDVIEWDIALOG_H

#include <QDialog>

namespace Ui {
class CredViewDialog;
}

class CredViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CredViewDialog(const QString& url,
                            const QString& login,
                            const QString& password,
                            QWidget *parent = nullptr);
    ~CredViewDialog();

private slots:
    void on_copyLoginButton_clicked();
    void on_copyPasswordButton_clicked();
    void on_closeButton_clicked();

private:
    Ui::CredViewDialog *ui;
    QString login_;
    QString password_;
};

#endif // CREDVIEWDIALOG_H
