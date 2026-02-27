#include "mainwindow.h"
#include "newpindialog.h"
#include <QApplication>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef _WIN32
    if (IsDebuggerPresent()) {
        QMessageBox::critical(nullptr,
                              "Предупреждение",
                              "Обнаружен отладчик. Приложение будет закрыто.");
        return 0;
    }
#endif
    NewPinDialog dlg;
    if (dlg.exec() != QDialog::Accepted){
        return 0;
    }
    MainWindow w(dlg.creds());
    w.show();
    return a.exec();
}
