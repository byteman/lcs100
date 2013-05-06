#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <cstdio>


static DeviceList devlist;
static unsigned char    gGroup = 0;
static unsigned int     gID = 1;
static std::string uploadFile = "";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->btnUpload->setEnabled(false);
    ui->btnReset->setEnabled(false);
    ui->btnAdjust->setEnabled(false);

}
bool MainWindow::updateID(void)
{
    bool ok = false;


    gID = ui->edtID->text ().toInt (&ok);
    if(ok)
    {
        gGroup = ui->edtGroup->text ().toInt (&ok);
        if(ok) return true;

    }
    return false;
}
MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnUpload_clicked()
{
    if(uploadFile.length () == 0)
    {
        QMessageBox::information (this,tr("错误提示"),tr("还没有选择升级文件"));
        return;
    }

    devlist.clear ();
    devlist.push_back (gID);
    //devlist.push_back (2);
    LedCtrl::get ().upload (uploadFile,devlist);
    ui->prgUpload->setRange (0,LedCtrl::get ().getUploadFilePacketNum ());

}
void MainWindow::callback(TEventParam* par)
{
#if 1
    if(par->event == EV_DATA)
    {
        QString strID;
        strID.sprintf ("%d",par->id);
        ui->lblID->setText (strID);
        ui->prgUpload->setValue (par->arg);
    }
    else if(par->event == EV_REQ)
    {
        QString strMsg;
        strMsg = tr("请求升级成功");
        ui->lblMsg->setText(strMsg);
    }
    else if(par->event == EV_VERIFY)
    {
        QString strMsg;
        strMsg = tr("校验成功");
        ui->lblMsg->setText(strMsg);
    }
#endif
}
static void LedEventProc(TEventParam* par,void* arg)
{
    if(arg != NULL)
    {
        MainWindow* mw = (MainWindow*)arg;
        mw->callback(par);
    }
}
void MainWindow::on_btnStart_clicked()
{
    if(!LedCtrl::get ().open ("/dev/ttyUSB0"))
    {

        LedCtrl::get ().setCallBack (LedEventProc,this);
        ui->btnUpload->setEnabled(true);
        ui->btnReset->setEnabled(true);
        ui->btnAdjust->setEnabled(true);
        QMessageBox::information (this,tr("led单灯控制器"),tr("串口打开失败!"));
    }
    else
    {
        QMessageBox::information (this,tr("led单灯控制器"),tr("启动成功!!"));
    }
}

void MainWindow::on_btnReset_clicked()
{
    updateID();
    LedCtrl::get().setDeviceReset (gID,gGroup);
}

void MainWindow::on_btnFile_clicked()
{
    QFileDialog *dlg = new QFileDialog(this);

    dlg->setWindowTitle(tr("Open Image"));
    dlg->setDirectory(".");
    dlg->setFilter(tr("upload Files(*.bin)"));
    if(dlg->exec() == QDialog::Accepted)
    {
        QString path = dlg->selectedFiles()[0];
        if(path.length () > 0 )
        {
            uploadFile = path.toStdString ();
        }
        QMessageBox::information(NULL, tr("Path"), tr("You selected ") + path);
    }
    else
    {
        QMessageBox::information(NULL, tr("Path"), tr("You didn't select any files."));
    }

}
