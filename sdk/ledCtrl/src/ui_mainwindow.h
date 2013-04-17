/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Tue Apr 9 15:21:14 2013
**      by: Qt User Interface Compiler version 4.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QTabWidget *tabWidget;
    QWidget *tabSetting;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *btnReset;
    QPushButton *btnStart;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_2;
    QLineEdit *lineEdit_2;
    QPushButton *btnAdjust;
    QWidget *tabUpload;
    QPushButton *btnUpload;
    QProgressBar *prgUpload;
    QPushButton *btnFile;
    QLabel *label_5;
    QLabel *lblID;
    QLabel *lblMsg;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label_3;
    QLineEdit *edtID;
    QLabel *label_4;
    QLineEdit *edtGroup;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(692, 476);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(0, 30, 691, 381));
        tabSetting = new QWidget();
        tabSetting->setObjectName(QString::fromUtf8("tabSetting"));
        verticalLayoutWidget = new QWidget(tabSetting);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 30, 151, 99));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(verticalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        lineEdit = new QLineEdit(verticalLayoutWidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        verticalLayout->addWidget(lineEdit);

        btnReset = new QPushButton(verticalLayoutWidget);
        btnReset->setObjectName(QString::fromUtf8("btnReset"));

        verticalLayout->addWidget(btnReset);

        btnStart = new QPushButton(tabSetting);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));
        btnStart->setGeometry(QRect(10, 260, 158, 32));
        verticalLayoutWidget_2 = new QWidget(tabSetting);
        verticalLayoutWidget_2->setObjectName(QString::fromUtf8("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(10, 140, 151, 101));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(verticalLayoutWidget_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout_2->addWidget(label_2);

        lineEdit_2 = new QLineEdit(verticalLayoutWidget_2);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));

        verticalLayout_2->addWidget(lineEdit_2);

        btnAdjust = new QPushButton(verticalLayoutWidget_2);
        btnAdjust->setObjectName(QString::fromUtf8("btnAdjust"));

        verticalLayout_2->addWidget(btnAdjust);

        tabWidget->addTab(tabSetting, QString());
        tabUpload = new QWidget();
        tabUpload->setObjectName(QString::fromUtf8("tabUpload"));
        btnUpload = new QPushButton(tabUpload);
        btnUpload->setObjectName(QString::fromUtf8("btnUpload"));
        btnUpload->setGeometry(QRect(70, 50, 158, 32));
        prgUpload = new QProgressBar(tabUpload);
        prgUpload->setObjectName(QString::fromUtf8("prgUpload"));
        prgUpload->setGeometry(QRect(70, 110, 471, 23));
        prgUpload->setValue(0);
        btnFile = new QPushButton(tabUpload);
        btnFile->setObjectName(QString::fromUtf8("btnFile"));
        btnFile->setGeometry(QRect(260, 50, 151, 31));
        label_5 = new QLabel(tabUpload);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(70, 150, 78, 21));
        lblID = new QLabel(tabUpload);
        lblID->setObjectName(QString::fromUtf8("lblID"));
        lblID->setGeometry(QRect(150, 150, 78, 21));
        lblMsg = new QLabel(tabUpload);
        lblMsg->setObjectName(QString::fromUtf8("lblMsg"));
        lblMsg->setGeometry(QRect(80, 240, 78, 21));
        tabWidget->addTab(tabUpload, QString());
        horizontalLayoutWidget = new QWidget(centralWidget);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(270, 0, 361, 51));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label_3 = new QLabel(horizontalLayoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout->addWidget(label_3);

        edtID = new QLineEdit(horizontalLayoutWidget);
        edtID->setObjectName(QString::fromUtf8("edtID"));

        horizontalLayout->addWidget(edtID);

        label_4 = new QLabel(horizontalLayoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout->addWidget(label_4);

        edtGroup = new QLineEdit(horizontalLayoutWidget);
        edtGroup->setObjectName(QString::fromUtf8("edtGroup"));

        horizontalLayout->addWidget(edtGroup);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 692, 30));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Led\345\215\225\347\201\257\346\265\213\350\257\225", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "\345\244\215\344\275\215\346\227\266\351\227\264\357\274\232", 0, QApplication::UnicodeUTF8));
        btnReset->setText(QApplication::translate("MainWindow", "\345\244\215\344\275\215\346\265\213\350\257\225", 0, QApplication::UnicodeUTF8));
        btnStart->setText(QApplication::translate("MainWindow", "\345\220\257\345\212\250", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "\350\260\203\345\205\211\345\200\274\357\274\232", 0, QApplication::UnicodeUTF8));
        btnAdjust->setText(QApplication::translate("MainWindow", "\350\260\203\345\205\211", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabSetting), QApplication::translate("MainWindow", "Tab 1", 0, QApplication::UnicodeUTF8));
        btnUpload->setText(QApplication::translate("MainWindow", "\345\215\207\347\272\247\346\265\213\350\257\225", 0, QApplication::UnicodeUTF8));
        btnFile->setText(QApplication::translate("MainWindow", "\351\200\211\346\213\251\345\215\207\347\272\247\346\226\207\344\273\266...", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("MainWindow", "\345\275\223\345\211\215ID\357\274\232", 0, QApplication::UnicodeUTF8));
        lblID->setText(QApplication::translate("MainWindow", "0", 0, QApplication::UnicodeUTF8));
        lblMsg->setText(QApplication::translate("MainWindow", "msg:", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabUpload), QApplication::translate("MainWindow", "Tab 2", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MainWindow", "\350\256\276\345\244\207ID\357\274\232", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MainWindow", "\347\273\204\345\217\267\357\274\232", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
