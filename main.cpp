#include "mainwindow.h"
#include "newpindialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    NewPinDialog dlg;
    if (dlg.exec() != QDialog::Accepted){
        return 0;
    }
    MainWindow w;
    w.show();
    return a.exec();
}
