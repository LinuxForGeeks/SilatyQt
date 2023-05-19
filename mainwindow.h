#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QCloseEvent>
#include <QAction>
#include <QMediaPlayer>

extern QString sel_locale;
extern QString version;
extern int cal_m;
extern int madhab_cal;
extern double latitude;
extern double longitude;
extern double Time_Zone;
extern int clock_format;
extern QString fajr_n;
extern QString shuruk_n;
extern QString dhuhr_n;
extern QString asr_n;
extern QString maghrib_n;
extern QString isha_n;
extern QString fajr_n_24;
extern QString shuruk_n_24;
extern QString dhuhr_n_24;
extern QString asr_n_24;
extern QString maghrib_n_24;
extern QString isha_n_24;
extern int currentHour;
extern int currentMinute;
extern int betweenHour;
extern int betweenMinute;
extern QAction *fajr_tr;
extern QAction *dhuhr_tr;
extern QAction *asr_tr;
extern QAction *maghrib_tr;
extern QAction *isha_tr;
extern QAction *time_left_tray;
extern QAction *location_tr;
extern QAction *home_tr;
extern int adjust_hijri_calendar;
extern bool show_time_left_icon;
extern bool enable_audio_notif;
extern int time_before_notification;
extern int fajr_azan;
extern int normal_azan;
extern bool hijri_calendar;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Ui::MainWindow *ui;
    void quit() {
        QApplication::quit();
        return;
    }
    void showHome() {
        sel_home();
        MainWindow::show();
    }
    void showQibla() {
        sel_qibla();
        MainWindow::show();
    }
    void showCalendar() {
        sel_calendar();
        MainWindow::show();
    }
    void showSettings() {
        sel_settings();
        MainWindow::show();
    }
    void showAbout() {
        sel_about();
        MainWindow::show();
    }

public slots:
    void get_locations();
    void get_hours_and_minutes();
    void set_locales();
    void sel_home();
    void sel_qibla();
    void sel_calendar();
    void sel_settings();
    void sel_about();
    void get_prayer();
    void calc_prayer(int, int, double, double, double);
    void call_prayer(QString, QString, int);
    void play_fajr_audio();
    void play_normal_audio();

    void save_settings_data();

private:
    QMediaPlayer *player;

protected:
    void closeEvent(QCloseEvent *event) override {
        event->ignore();
        hide();
    }
};

#endif // MAINWINDOW_H
