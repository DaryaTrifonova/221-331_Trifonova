#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include "crypto_utils.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QVector<Cred>& creds, QWidget *parent = nullptr);
    ~MainWindow();

private:

    bool loadCredsFromJsonFile(const QString &path);
    void fillTable();
    void applyFilter(const QString &query);
    Ui::MainWindow *ui;
    QVector<Cred> creds_;

private slots:
    void onCredDoubleClicked(int row, int column);
};
#endif // MAINWINDOW_H
