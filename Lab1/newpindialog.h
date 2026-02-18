#ifndef NEWPINDIALOG_H
#define NEWPINDIALOG_H

#include <QDialog>
#include <QObject>

namespace Ui {
class NewPinDialog;
}

class NewPinDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewPinDialog(QWidget *parent = nullptr);
    ~NewPinDialog();

private slots:
    void on_unlockButton_clicked();

private:
    Ui::NewPinDialog *ui;
};

#endif // NEWPINDIALOG_H
