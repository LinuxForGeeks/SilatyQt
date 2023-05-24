#include "mainwindow.h"
#include <QApplication>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QObject>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <SingleApplication.h>

QSystemTrayIcon *tray;

int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv);
    MainWindow w;
    // set window icon

    a.setWindowIcon(QIcon(QString(":/icons/silaty_icon")));

    // make tray
    QMenu *tray_menu = new QMenu("SilatyTray");

    //QSystemTrayIcon *tray
    tray = new QSystemTrayIcon();
    tray->setIcon(QIcon(QString(":/icons/silaty_tray")));

    QString val;
    QFile file;
    file.setFileName(":/lang/"+sel_locale);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    QJsonDocument JsonDoc = QJsonDocument::fromJson(val.toUtf8());

    home_tr = new QAction("");
    QObject::connect(home_tr, &QAction::triggered, &w, &MainWindow::showHome);
    home_tr->setIcon(QIcon(QString(":/sidebar/Home")));
    tray_menu->addAction(home_tr);

    tray_menu->addSeparator();

    location_tr = new QAction("");
    location_tr->setEnabled(false);
    location_tr->setIcon(QIcon(QString(":/sidebar/Qibla")));
    tray_menu->addAction(location_tr);

    tray_menu->addSeparator();

    fajr_tr = new QAction("");
    fajr_tr->setEnabled(false);
    tray_menu->addAction(fajr_tr);

    dhuhr_tr = new QAction("");
    dhuhr_tr->setEnabled(false);
    tray_menu->addAction(dhuhr_tr);

    asr_tr = new QAction("");
    asr_tr->setEnabled(false);
    tray_menu->addAction(asr_tr);

    maghrib_tr = new QAction("");
    maghrib_tr->setEnabled(false);
    tray_menu->addAction(maghrib_tr);

    isha_tr = new QAction("");
    isha_tr->setEnabled(false);
    tray_menu->addAction(isha_tr);

    tray_menu->addSeparator();

    time_left_tray = new QAction("");
    time_left_tray->setEnabled(false);
    tray_menu->addAction(time_left_tray);

    tray_menu->addSeparator();

    QAction *about = new QAction(JsonDoc.object().value("tAbout").toString());
    QObject::connect(about, &QAction::triggered, &w, &MainWindow::showAbout);
    about->setIcon(QIcon(QString(":/sidebar/About")));
    tray_menu->addAction(about);

    QAction *settings = new QAction(JsonDoc.object().value("tSettings").toString());
    QObject::connect(settings, &QAction::triggered, &w, &MainWindow::showSettings);
    settings->setIcon(QIcon(QString(":/sidebar/Settings")));
    tray_menu->addAction(settings);

    QAction *quit = new QAction(JsonDoc.object().value("tQuit").toString());
    QObject::connect(quit, &QAction::triggered, &w, &MainWindow::quit);
    quit->setIcon(QIcon(QString(":/icons/Quit")));
    tray_menu->addAction(quit);

    tray->setContextMenu(tray_menu);
    tray->show();
    tray->setVisible(true);

    QObject::connect(tray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),&w,SLOT(showHide(QSystemTrayIcon::ActivationReason)));

    return a.exec();
}

void MainWindow::showHide(QSystemTrayIcon::ActivationReason r)
{
    if (r == QSystemTrayIcon::Trigger)
    {
        tray->contextMenu()->popup(QCursor::pos());
    }
}
