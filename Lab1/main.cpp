#include "mainwindow.h"
#include "newpindialog.h"
#include <QApplication>
#include <QCryptographicHash>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif
#include <QMessageBox>

typedef long long QWORD;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWORD moduleBase = (QWORD)GetModuleHandle(NULL);
    QWORD textAddress = moduleBase + 0x1000;

    PIMAGE_DOS_HEADER pIDH = reinterpret_cast<PIMAGE_DOS_HEADER>(moduleBase);
    qDebug() << QByteArray(reinterpret_cast<char*>(&pIDH->e_magic),2);
    PIMAGE_NT_HEADERS pINH = reinterpret_cast<PIMAGE_NT_HEADERS>(moduleBase + pIDH->e_lfanew);
    size_t sizeOfCode = pINH->OptionalHeader.SizeOfCode;
    qDebug() << QByteArray(reinterpret_cast<char*>(&pINH->Signature),4);

    QByteArray calculatedHash = QCryptographicHash::hash(
        QByteArrayView(reinterpret_cast<char*>(textAddress), sizeOfCode),
        QCryptographicHash::Sha256);
    qDebug() << "calculatedHash = " << calculatedHash.toHex();

    QByteArray requiredHash =
        QByteArray::fromHex("afd2d19c6f36fbbd8f60993b61c6d6e59fae89620d844a7668114675fd7f9698");
    if(calculatedHash != requiredHash){
        QMessageBox::critical(
            nullptr,
            "Внимание!",
            "Обнаружена модификация приложения.");
        return 0;
    }

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
