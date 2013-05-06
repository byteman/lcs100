#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "LedCtrl.h"
namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void callback(TEventParam* par);

private slots:
    void on_btnUpload_clicked();

    void on_btnStart_clicked();

    void on_btnReset_clicked();

    void on_btnFile_clicked();

private:
    bool updateID(void);

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
