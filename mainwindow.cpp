#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QObject>
#include <QFile>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QDateTime>
#include <QDebug>
#include <QBuffer>
#include <QFile>
#include <QAudioOutput>
#include <QSystemTrayIcon>
#include <QtGui>
#include <QSoundEffect>
#include <iostream>
#include <string>
#include <vector>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QSettings>

using namespace std;

QString sel_locale = "en_US";
QString version = "1.6";
double latitude;
double longitude;
double Time_Zone;
int clock_format;
int madhab_cal;
int cal_m;
QString fajr_n;
QString shuruk_n;
QString dhuhr_n;
QString asr_n;
QString maghrib_n;
QString isha_n;
QString fajr_n_24;
QString shuruk_n_24;
QString dhuhr_n_24;
QString asr_n_24;
QString maghrib_n_24;
QString isha_n_24;
int currentHour;
int currentMinute;
int betweenHour;
int betweenMinute;
int NextPrayer;
QJsonDocument JsonDoc;
QAction *fajr_tr;
QAction *dhuhr_tr;
QAction *asr_tr;
QAction *maghrib_tr;
QAction *isha_tr;
QAction *time_left_tray;
QAction *location_tr;
QAction *home_tr;
bool repeat_salah_func = false;
QString notif;
QSettings saved_settings("SilatyTeam", "Silaty");
QMediaPlayer player;

int fajr_azan;
int normal_azan;
bool hijri_calendar;
bool aot;
bool enable_audio_notif;
int time_before_notification;

int call_reminder = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    player = new QMediaPlayer(this);

    const QString LanguageArray[4] = {"en_US", "ar_FR", "fr_FR", "es_ES"};

    ui->sLc->setCurrentIndex(saved_settings.value("language").toInt());
    sel_locale = LanguageArray[saved_settings.value("language").toInt()];

    // setup
    sel_home();
    set_locales();

    ui->sCFc->setCurrentIndex(saved_settings.value("clock_format").toInt());
    clock_format = ui->sCFc->currentIndex();
    ui->cHijri->setChecked(saved_settings.value("hijri_calendar").toBool());
    hijri_calendar = ui->cHijri->isChecked();
    ui->sAOTc->setChecked(saved_settings.value("always_on_top").toBool());
    aot = ui->sAOTc->isChecked();
    if (saved_settings.contains("enable_audio_notif")) {
        ui->sEANc->setChecked(saved_settings.value("enable_audio_notif").toBool());
    } else {
        saved_settings.setValue("enable_audio_notif", true);
        ui->sEANc->setChecked(saved_settings.value("enable_audio_notif").toBool());
    }
    enable_audio_notif = ui->sEANc->isChecked();
    if (hijri_calendar == true) {
        ui->cCalendar->setCalendar(QCalendar(QCalendar::System::IslamicCivil));
    } else {
        ui->cCalendar->setCalendar(QCalendar(QCalendar::System::Gregorian));
    }
    if (aot == true) {
        this->setWindowFlags(Qt::WindowStaysOnTopHint);
    }
    ui->sFAc->setCurrentIndex(saved_settings.value("fajr_azan").toInt());
    ui->sNAc->setCurrentIndex(saved_settings.value("normal_azan").toInt());
    ui->sMadhabc->setCurrentIndex(saved_settings.value("madhab").toInt());
    ui->sCMc->setCurrentIndex(saved_settings.value("calc_method").toInt());
    if (saved_settings.contains("time_zone")) {
        ui->sTZd->setValue(saved_settings.value("time_zone").toDouble());
    } else {
        saved_settings.setValue("time_zone", 2.00);
        ui->sTZd->setValue(saved_settings.value("time_zone").toDouble());
    }
    if (saved_settings.contains("time_before_notification")) {
        ui->sTBNs->setValue(saved_settings.value("time_before_notification").toInt());
    } else {
        saved_settings.setValue("time_before_notification", 10);
        ui->sTBNs->setValue(saved_settings.value("time_before_notification").toInt());
    }
    time_before_notification = ui->sTBNs->value();
    fajr_azan = ui->sFAc->currentIndex();
    normal_azan = ui->sNAc->currentIndex();
    Time_Zone = ui->sTZd->value();
    madhab_cal = ui->sMadhabc->currentIndex();
    cal_m = ui->sCMc->currentIndex();
    if (aot == true) {
        this->setWindowFlags(Qt::WindowStaysOnTopHint);
    }
    get_prayer();

    ui->sSMc->setChecked(saved_settings.value("start_minimized").toBool());
    if (ui->sSMc->isChecked() == false) {
        showHome();
    }

    // connect buttons
    QObject::connect(ui->homeButton, SIGNAL(clicked()), this, SLOT(sel_home()));
    QObject::connect(ui->compassButton, SIGNAL(clicked()), this, SLOT(sel_qibla()));
    QObject::connect(ui->calendarButton, SIGNAL(clicked()), this, SLOT(sel_calendar()));
    QObject::connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(sel_settings()));
    QObject::connect(ui->aboutButton, SIGNAL(clicked()), this, SLOT(sel_about()));
    QObject::connect(ui->sCc, SIGNAL(currentIndexChanged(int)), this, SLOT(get_prayer()));
    QObject::connect(ui->sCFc, SIGNAL(currentIndexChanged(int)), this, SLOT(save_settings_data()));
    QObject::connect(ui->sFAc, SIGNAL(currentIndexChanged(int)), this, SLOT(save_settings_data()));
    QObject::connect(ui->sNAc, SIGNAL(currentIndexChanged(int)), this, SLOT(save_settings_data()));
    QObject::connect(ui->sFAt, SIGNAL(clicked(bool)), this, SLOT(play_fajr_audio()));
    QObject::connect(ui->sNAt, SIGNAL(clicked(bool)), this, SLOT(play_normal_audio()));
    QObject::connect(ui->sTZd, SIGNAL(valueChanged(double)), this, SLOT(save_settings_data()));
    QObject::connect(ui->sLc, SIGNAL(currentIndexChanged(int)), this, SLOT(save_settings_data()));
    QObject::connect(ui->sMadhabc, SIGNAL(currentIndexChanged(int)), this, SLOT(save_settings_data()));
    QObject::connect(ui->sCMc, SIGNAL(currentIndexChanged(int)), this, SLOT(save_settings_data()));
    QObject::connect(ui->cHijri, SIGNAL(stateChanged(int)), this, SLOT(save_settings_data()));
    QObject::connect(ui->sAOTc, SIGNAL(stateChanged(int)), this, SLOT(save_settings_data()));
    QObject::connect(ui->sSMc, SIGNAL(stateChanged(int)), this, SLOT(save_settings_data()));
    QObject::connect(ui->sEANc, SIGNAL(stateChanged(int)), this, SLOT(save_settings_data()));
    QObject::connect(ui->sTBNs, SIGNAL(valueChanged(int)), this, SLOT(save_settings_data()));
}

void MainWindow::save_settings_data() {
    clock_format = ui->sCFc->currentIndex();
    saved_settings.setValue("clock_format", ui->sCFc->currentIndex());
    fajr_azan = ui->sFAc->currentIndex();
    normal_azan = ui->sNAc->currentIndex();
    Time_Zone = ui->sTZd->value();
    madhab_cal = ui->sMadhabc->currentIndex();
    cal_m = ui->sCMc->currentIndex();
    hijri_calendar = ui->cHijri->isChecked();
    enable_audio_notif = ui->sEANc->isChecked();
    time_before_notification = ui->sTBNs->value();
    saved_settings.setValue("fajr_azan", ui->sFAc->currentIndex());
    saved_settings.setValue("normal_azan", ui->sNAc->currentIndex());
    saved_settings.setValue("time_zone", ui->sTZd->value());
    saved_settings.setValue("language", ui->sLc->currentIndex());
    saved_settings.setValue("madhab", ui->sMadhabc->currentIndex());
    saved_settings.setValue("calc_method", ui->sCMc->currentIndex());
    saved_settings.setValue("hijri_calendar", ui->cHijri->isChecked());
    saved_settings.setValue("always_on_top", ui->sAOTc->isChecked());
    saved_settings.setValue("start_minimized", ui->sSMc->isChecked());
    saved_settings.setValue("enable_audio_notif", ui->sEANc->isChecked());
    saved_settings.setValue("time_before_notification", ui->sTBNs->value());
    if (hijri_calendar == true) {
        ui->cCalendar->setCalendar(QCalendar(QCalendar::System::IslamicCivil));
    } else {
        ui->cCalendar->setCalendar(QCalendar(QCalendar::System::Gregorian));
    }
    get_prayer();
}

void MainWindow::set_locales() {
    // read locale file
    QString val;
    QFile file;
    file.setFileName(":/lang/"+sel_locale);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    JsonDoc = QJsonDocument::fromJson(val.toUtf8());
    // set language
    QLocale::setDefault(QLocale(sel_locale));
    // get hours and minutes
    QTimer* timer2 = new QTimer();
    timer2->setInterval(100);
    connect(timer2, &QTimer::timeout, this, [=](){
        get_hours_and_minutes();
    });
    timer2->start();
    ui->hFajrL->setText("   " + JsonDoc.object().value("hFajrL").toString());
    ui->hShurukL->setText("    " + JsonDoc.object().value("hShurukL").toString());
    ui->hDhuhrL->setText("   " + JsonDoc.object().value("hDhuhrL").toString());
    ui->hAsrL->setText("    " + JsonDoc.object().value("hAsrL").toString());
    ui->hMaghribL->setText("   " + JsonDoc.object().value("hMaghribL").toString());
    ui->hIshaL->setText("    " + JsonDoc.object().value("hIshaL").toString());
    ui->cHijri->setText(JsonDoc.object().value("cHijri").toString());
    ui->sSystem->setText(JsonDoc.object().value("sSystem").toString());
    ui->sSM->setText("   " + JsonDoc.object().value("sSM").toString());
    ui->sCF->setText("   " + JsonDoc.object().value("sCF").toString());
    ui->sL->setText("   " + JsonDoc.object().value("sL").toString());
    ui->sNotifications->setText(JsonDoc.object().value("sNotifications").toString());
    ui->sEAN->setText("   " + JsonDoc.object().value("sEAN").toString());
    ui->sTBN->setText("   " + JsonDoc.object().value("sTBN").toString());
    ui->sFA->setText("   " + JsonDoc.object().value("sFA").toString());
    ui->sNA->setText("   " + JsonDoc.object().value("sNA").toString());
    ui->sJurisprudence->setText(JsonDoc.object().value("sJurisprudence").toString());
    ui->sCM->setText("   " + JsonDoc.object().value("sCM").toString());
    ui->sMadhab->setText("   " + JsonDoc.object().value("sMadhab").toString());
    ui->sLocation->setText(JsonDoc.object().value("sLocation").toString());
    ui->sCity->setText("   " + JsonDoc.object().value("sCity").toString());
    ui->sLatitude->setText("   " + JsonDoc.object().value("sLatitude").toString());
    ui->sLongitude->setText("   " + JsonDoc.object().value("sLongitude").toString());
    ui->sTZ->setText("   " + JsonDoc.object().value("sTZ").toString());
    ui->sCFc->setItemText(0, JsonDoc.object().value("s12h").toString());
    ui->sCFc->setItemText(1, JsonDoc.object().value("s24h").toString());
    ui->sLc->setItemText(0, JsonDoc.object().value("sEnglish").toString());
    ui->sLc->setItemText(1, JsonDoc.object().value("sArabic").toString());
    ui->sLc->setItemText(2, JsonDoc.object().value("sFrench").toString());
    ui->sLc->setItemText(3, JsonDoc.object().value("sSpanish").toString());
    ui->sCMc->setItemText(0, JsonDoc.object().value("sMakkah").toString());
    ui->sCMc->setItemText(1, JsonDoc.object().value("sEgypt").toString());
    ui->sCMc->setItemText(2, JsonDoc.object().value("sKarachi").toString());
    ui->sCMc->setItemText(3, JsonDoc.object().value("sISNA").toString());
    ui->sCMc->setItemText(4, JsonDoc.object().value("sMWL").toString());
    ui->sMadhabc->setItemText(0, JsonDoc.object().value("sDefault").toString());
    ui->sMadhabc->setItemText(1, JsonDoc.object().value("sHanafi").toString());
    ui->aSilaty->setText(QString(JsonDoc.object().value("Silaty").toString()));
    ui->aVersion->setText(version);
    ui->aDesc->setText(QString(JsonDoc.object().value("aDesc").toString()));
    ui->aGPP->setText("<a href=\"https://github.com/LinuxForGeeks/SilatyQt\">"+QString(JsonDoc.object().value("aGPP").toString())+"</a>");
    ui->aGPP->setTextFormat(Qt::RichText);
    ui->aGPP->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->aGPP->setOpenExternalLinks(true);
    ui->aCST->setText(QString(JsonDoc.object().value("aCST").toString()));
    ui->CreateCredits->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->CreateCredits->setOpenExternalLinks(true);
    ui->ArtworkCredits->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->ArtworkCredits->setOpenExternalLinks(true);
    get_locations();
    ui->cCalendar->setLocale(QLocale(sel_locale));
    if (sel_locale == "ar_FR") {
        ui->mainLayout->setLayoutDirection(Qt::RightToLeft);
        ui->home->setLayoutDirection(Qt::RightToLeft);
        ui->compass->setLayoutDirection(Qt::RightToLeft);
        ui->calendar->setLayoutDirection(Qt::RightToLeft);
        ui->settings->setLayoutDirection(Qt::RightToLeft);
        ui->about->setLayoutDirection(Qt::RightToLeft);
        ui->cHijri->setLayoutDirection(Qt::LeftToRight);
        ui->sTBNs->setAlignment(Qt::AlignRight);
        ui->sLatituded->setAlignment(Qt::AlignRight);
        ui->sLongituded->setAlignment(Qt::AlignRight);
        ui->sTZd->setAlignment(Qt::AlignRight);
        ui->aWc->setLayoutDirection(Qt::LeftToRight);
        ui->aWl->setLayoutDirection(Qt::LeftToRight);
        ui->sAOT->setAlignment(Qt::AlignRight);
    }
    QTimer* timer1 = new QTimer();
    timer1->setInterval(100);
    connect(timer1, &QTimer::timeout, this, [=](){
        QCalendar calendar(QCalendar::System::IslamicCivil);
        QDate today = QDate::currentDate();
        QCalendar::YearMonthDay ymd = calendar.partsFromDate(today);
        QString HijriMonth = QString(calendar.monthName(QLocale(QLocale().name()), ymd.month, ymd.year, QLocale::LongFormat));
        QString Date_Time_Info = QTime::currentTime().toString("HH:mm") + QString(" - ") + QLocale().toString(QDate::currentDate(), "dddd") + QString(" , ") + QString::number(ymd.day) + QString(" ") + QString(HijriMonth) + QString(" ") + QString::number(ymd.year) + QString(" / ") + QString(QDate::currentDate().toString("dd")) + QLocale().toString(QDate::currentDate(), " MMMM ") + QString(QDate::currentDate().toString("yyyy"));
        ui->hDate->setText(Date_Time_Info);
        if (hijri_calendar == true) {
            ui->cDate->setText(QLocale().toString(QDate::currentDate(), "dddd")+", "+QString::number(ymd.day) + QString(" ") + QString(HijriMonth) + QString(" ") + QString::number(ymd.year));
        } else {
            ui->cDate->setText(QLocale().toString(QDate::currentDate(), "dddd")+", "+QString(QDate::currentDate().toString("dd")) + QLocale().toString(QDate::currentDate(), " MMMM ") + QString(QDate::currentDate().toString("yyyy")));
        }
    });
    timer1->start();
    call_prayer("none", "Normal", 4);
}

void MainWindow::get_prayer() {
    saved_settings.setValue("location_index", ui->sCc->currentIndex());
    latitude = 0.000000;
    longitude = 0.000000;

    QString selected_country = "";
    selected_country = ui->sCc->currentText().split("/")[0];

    QString selected_city = "";
    selected_city = ui->sCc->currentText().split("/")[1];

    QFile inFile(":/data/Locations");
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug( "Failed to open file for reading." );
    }

    QDomDocument document;
    if( !document.setContent( &inFile ) )
    {
        qDebug( "Failed to parse the file into a DOM tree." );
        inFile.close();
    }

    inFile.close();
    QDomElement documentElement = document.documentElement();
    QDomNodeList elements = documentElement.elementsByTagName("region");

    for (int i = 0; i < elements.size(); i++)
    {
        QDomElement region_name = elements.at(i).toElement().firstChildElement("name");
        QDomNodeList elements2 = elements.at(i).toElement().elementsByTagName("country");
        for (int i2 = 0; i2 < elements2.size(); i2++)
        {
            QDomElement country_name = elements2.at(i2).toElement().firstChildElement("name");
            QDomNodeList elements3 = elements2.at(i2).toElement().elementsByTagName("city");
            for (int i3 = 0; i3 < elements3.size(); i3++)
            {
                QDomElement city_name = elements3.at(i3).toElement().firstChildElement("name");
                if (country_name.text() == selected_country) {
                    if (city_name.text() == selected_city) {
                        QDomElement coordinates_val = elements3.at(i3).toElement().firstChildElement("coordinates");
                        latitude = stod(coordinates_val.text().split(" ")[0].toStdString());
                        longitude = stod(coordinates_val.text().split(" ")[1].toStdString());
                        ui->sLatituded->setValue(latitude);
                        ui->sLongituded->setValue(longitude);
                    }
                }
            }
        }
    }

    cal_m = ui->sMadhabc->currentIndex();

    calc_prayer(cal_m, madhab_cal, latitude, longitude, Time_Zone);
    ui->hFajrT->setText(fajr_n + "   ");
    ui->hShurukT->setText(shuruk_n + "    ");
    ui->hDhuhrT->setText(dhuhr_n + "   ");
    ui->hAsrT->setText(asr_n + "    ");
    ui->hMaghribT->setText(maghrib_n + "   ");
    ui->hIshaT->setText(isha_n + "    ");
}

void MainWindow::sel_home() {
    ui->sB->setContentsMargins(6,0,6,55);
    setFixedSize(429, 241);
    ui->widgets->setCurrentIndex(0);
    ui->homeButton->setChecked(true);
    ui->compassButton->setChecked(false);
    ui->calendarButton->setChecked(false);
    ui->settingsButton->setChecked(false);
    ui->aboutButton->setChecked(false);
}

void MainWindow::sel_qibla() {
    ui->sB->setContentsMargins(6,0,6,145);
    setFixedSize(429, 331);
    ui->widgets->setCurrentIndex(1);
    ui->homeButton->setChecked(false);
    ui->compassButton->setChecked(true);
    ui->calendarButton->setChecked(false);
    ui->settingsButton->setChecked(false);
    ui->aboutButton->setChecked(false);
}

void MainWindow::sel_calendar() {
    ui->sB->setContentsMargins(6,0,6,120);
    setFixedSize(429, 308);
    ui->widgets->setCurrentIndex(2);
    ui->homeButton->setChecked(false);
    ui->compassButton->setChecked(false);
    ui->calendarButton->setChecked(true);
    ui->settingsButton->setChecked(false);
    ui->aboutButton->setChecked(false);
}

void MainWindow::sel_settings() {
    ui->sB->setContentsMargins(6,0,6,235);
    setFixedSize(504, 420);
    ui->widgets->setCurrentIndex(3);
    ui->homeButton->setChecked(false);
    ui->compassButton->setChecked(false);
    ui->calendarButton->setChecked(false);
    ui->settingsButton->setChecked(true);
    ui->aboutButton->setChecked(false);
}

void MainWindow::sel_about() {
    ui->sB->setContentsMargins(6,0,6,285);
    setFixedSize(504, 475);
    ui->widgets->setCurrentIndex(4);
    ui->homeButton->setChecked(false);
    ui->compassButton->setChecked(false);
    ui->calendarButton->setChecked(false);
    ui->settingsButton->setChecked(false);
    ui->aboutButton->setChecked(true);
}

void MainWindow::get_hours_and_minutes() {
    // get next prayer
    const QString prayer_list[5] = {fajr_n_24, dhuhr_n_24, asr_n_24, maghrib_n_24, isha_n_24};
    for(int i = 0; i < 5; ++i) {
        if (QTime::currentTime() < QTime::fromString(prayer_list[i],"HH:mm")) {
            NextPrayer = i;
            i = 5;
        } else if (QTime::currentTime() > QTime::fromString(isha_n_24,"HH:mm")){
            NextPrayer = 0;
            i = 5;
        }
    }
    // get time left
    currentHour = QString(QTime::currentTime().toString("HH:mm")).split(":")[0].toInt();
    currentMinute = QString(QTime::currentTime().toString("HH:mm")).split(":")[1].toInt();
    int NextHour = QString(prayer_list[NextPrayer]).split(":")[0].toInt();
    int NextMinute = QString(prayer_list[NextPrayer]).split(":")[1].toInt();
    betweenHour = NextHour-currentHour;
    betweenMinute = NextMinute-currentMinute;
    int RemindHour = QString(prayer_list[NextPrayer]).split(":")[0].toInt();
    int RemindMinute = QString(prayer_list[NextPrayer]).split(":")[1].toInt()-time_before_notification;
    if (betweenMinute < 0) {
        betweenMinute += 60;
        betweenHour -= 1;
    }
    if (betweenHour < 0) {
        betweenHour += 24;
    }
    if (RemindMinute < 0) {
        RemindMinute += 60;
        RemindHour -= 1;
    }
    if (RemindHour < 0) {
        RemindHour += 24;
    }
    const QString prayer_names[5] = {JsonDoc.object().value("hFajrL").toString(), JsonDoc.object().value("hDhuhrL").toString(), JsonDoc.object().value("hAsrL").toString(), JsonDoc.object().value("hMaghribL").toString(), JsonDoc.object().value("hIshaL").toString()};
    if (betweenHour == 0) {
        this->setWindowTitle(JsonDoc.object().value("trUntilp").toString().replace("hrm", JsonDoc.object().value("trMinutes").toString().replace("mns", QString::number(betweenMinute))).replace("prayer", prayer_names[NextPrayer]));
        time_left_tray->setText(JsonDoc.object().value("trUntilp").toString().replace("hrm", JsonDoc.object().value("trMinutes").toString().replace("mns", QString::number(betweenMinute))).replace("prayer", prayer_names[NextPrayer]));
    } else if (betweenMinute == 0) {
        this->setWindowTitle(JsonDoc.object().value("trUntilp").toString().replace("hrm", JsonDoc.object().value("trHours").toString().replace("hrs", QString::number(betweenHour))).replace("prayer", prayer_names[NextPrayer]));
        time_left_tray->setText(JsonDoc.object().value("trUntilp").toString().replace("hrm", JsonDoc.object().value("trHours").toString().replace("hrs", QString::number(betweenHour))).replace("prayer", prayer_names[NextPrayer]));
    } else {
        this->setWindowTitle(JsonDoc.object().value("trUntilp").toString().replace("hrm", JsonDoc.object().value("trHaM").toString().replace("hrs", QString::number(betweenHour)).replace("mns", QString::number(betweenMinute))).replace("prayer", prayer_names[NextPrayer]));
        time_left_tray->setText(JsonDoc.object().value("trUntilp").toString().replace("hrm", JsonDoc.object().value("trHaM").toString().replace("hrs", QString::number(betweenHour)).replace("mns", QString::number(betweenMinute))).replace("prayer", prayer_names[NextPrayer]));
    }
    // Reminder
    if (currentHour == RemindHour && currentMinute == RemindMinute) {
        if (call_reminder == 0) {
            call_prayer(prayer_names[NextPrayer], "Normal", 3);
            call_reminder = 1;
        }
    } else {
        call_reminder = 0;
    }

    fajr_tr->setText(" "+ui->hFajrL->text() + ":- " + ui->hFajrT->text());
    dhuhr_tr->setText(" "+ui->hDhuhrL->text() + ":- " + ui->hDhuhrT->text());
    asr_tr->setText(ui->hAsrL->text() + ":- " + ui->hAsrT->text());
    maghrib_tr->setText(" "+ui->hMaghribL->text() + ":- " + ui->hMaghribT->text());
    isha_tr->setText(ui->hIshaL->text() + ":- " + ui->hIshaT->text());
    location_tr->setText(JsonDoc.object().value("sLocation").toString()+": "+ ui->sCc->currentText().split("/")[1]);
    home_tr->setText(ui->cDate->text());
    tray->setToolTip(time_left_tray->text());
    // highlight next prayer
    ui->hFajrL->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
    ui->hFajrT->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
    ui->hDhuhrL->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
    ui->hDhuhrT->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
    ui->hAsrL->setStyleSheet("");
    ui->hAsrT->setStyleSheet("");
    ui->hMaghribL->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
    ui->hMaghribT->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
    ui->hIshaL->setStyleSheet("");
    ui->hIshaT->setStyleSheet("");
    if (QString(QTime::currentTime().toString("HH:mm")) == fajr_n_24) {
        if (repeat_salah_func == false) {
            call_prayer(prayer_names[0], "Fajr", 0);
            repeat_salah_func = true;
        }
    } else if (QString(QTime::currentTime().toString("HH:mm")) == dhuhr_n_24) {
        if (repeat_salah_func == false) {
            call_prayer(prayer_names[1], "Dhuhr", 0);
            repeat_salah_func = true;
        }
    } else if (QString(QTime::currentTime().toString("HH:mm")) == asr_n_24) {
        if (repeat_salah_func == false) {
            call_prayer(prayer_names[2], "Asr", 0);
            repeat_salah_func = true;
        }
    } else if (QString(QTime::currentTime().toString("HH:mm")) == maghrib_n_24) {
        if (repeat_salah_func == false) {
            call_prayer(prayer_names[3], "Maghrib", 0);
            repeat_salah_func = true;
        }
    } else if (QString(QTime::currentTime().toString("HH:mm")) == isha_n_24) {
        if (repeat_salah_func == false) {
            call_prayer(prayer_names[4], "Isha", 0);
            repeat_salah_func = true;
        }
    } else {
        repeat_salah_func = false;
    }
    if (NextPrayer == 0) {
        ui->hFajrL->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); color: #55c1ec; border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
        ui->hFajrT->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); color: #55c1ec; border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
    }
    if (NextPrayer == 1) {
        ui->hDhuhrL->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); color: #55c1ec; border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
        ui->hDhuhrT->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); color: #55c1ec; border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
    }
    if (NextPrayer == 2) {
        ui->hAsrL->setStyleSheet("color: #55c1ec;");
        ui->hAsrT->setStyleSheet("color: #55c1ec;");
    }
    if (NextPrayer == 3) {
        ui->hMaghribL->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); color: #55c1ec; border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
        ui->hMaghribT->setStyleSheet("background-color: rgba(0, 0, 0, 7.5%); color: #55c1ec; border-width: 1px; border-style: solid; border-color: rgba(0, 0, 0, 15%) rgba(0, 0, 0, 0) rgba(0, 0, 0, 15%)  rgba(0, 0, 0, 0);");
    }
    if (NextPrayer == 4) {
        ui->hIshaL->setStyleSheet("color: #55c1ec;");
        ui->hIshaT->setStyleSheet("color: #55c1ec;");
    }
}

void MainWindow::get_locations() {
    QFile inFile(":/data/Locations");
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug( "Failed to open file for reading." );
    }

    QDomDocument document;
    if( !document.setContent( &inFile ) )
    {
        qDebug( "Failed to parse the file into a DOM tree." );
        inFile.close();
    }

    inFile.close();
    QDomElement documentElement = document.documentElement();
    QDomNodeList elements = documentElement.elementsByTagName("region");

    for (int i = 0; i < elements.size()+1; i++)
    {
        if (i < elements.size()) {
            QDomElement region_name = elements.at(i).toElement().firstChildElement("name");
            QDomNodeList elements2 = elements.at(i).toElement().elementsByTagName("country");
            for (int i2 = 0; i2 < elements2.size(); i2++)
            {
                QDomElement country_name = elements2.at(i2).toElement().firstChildElement("name");
                QDomNodeList elements3 = elements2.at(i2).toElement().elementsByTagName("city");
                for (int i3 = 0; i3 < elements3.size(); i3++)
                {
                    QDomElement city_name = elements3.at(i3).toElement().firstChildElement("name");
                    ui->sCc->addItem(country_name.text()+"/"+city_name.text());
                }
            }
        } else {
            ui->sCc->setCurrentIndex(saved_settings.value("location_index").toInt());
            get_prayer();
        }
    }
}

void MainWindow::call_prayer(QString prayer_na, QString prayer_en_name, int send_notif) {
    QString notif = JsonDoc.object().value("ITFP").toString().replace("name", prayer_na);
    QString until_notif = JsonDoc.object().value("trUntilp").toString().replace("prayer", prayer_na).replace("hrm", QString::number(time_before_notification)+" Minutes");
    QSystemTrayIcon notification = new QSystemTrayIcon();
    notification.setIcon(QIcon(QString(":/icons/silaty_png")));
    if (send_notif == 0) {
        notification.show();
        notification.setVisible(true);
        notification.showMessage(JsonDoc.object().value("nPTFN").toString().replace("name", prayer_na), notif, QIcon(QString(":/icons/silaty_png")));
    }
    if (send_notif == 3) {
        notification.show();
        notification.setVisible(true);
        notification.showMessage(JsonDoc.object().value("nGR").toString(), until_notif, QIcon(QString(":/icons/silaty_png")));
    }
    auto audioOutput = new QAudioOutput(this);
    //auto player = new QMediaPlayer(this);
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(100);
    if (send_notif == 0) {
        if (enable_audio_notif == true) {
            if (prayer_en_name == "Fajr") {
                player->setSource(QUrl("qrc:/audio/Fajr_"+ui->sFAc->currentText().replace(" ", "_")));
                player->play();
            } else {
                player->setSource(QUrl("qrc:/audio/Normal_"+ui->sNAc->currentText().replace(" ", "_")));
                player->play();
            }
        }
    } else if (send_notif == 1) {
        if (prayer_en_name == "Fajr") {
            player->setSource(QUrl("qrc:/audio/Fajr_"+ui->sFAc->currentText().replace(" ", "_")));
            player->play();
        } else {
            player->setSource(QUrl("qrc:/audio/Normal_"+ui->sNAc->currentText().replace(" ", "_")));
            player->play();
        }
    }
    if (send_notif == 2) {
        player->stop();
    }
}

void MainWindow::play_fajr_audio() {
    if (ui->sFAt->text() == "▶️") {
        ui->sFAt->setText("⏹");
        call_prayer("none", "Fajr", 1);
    } else if (ui->sFAt->text() == "⏹") {
        ui->sFAt->setText("▶️");
        call_prayer("none", "Fajr", 2);
    }
}

void MainWindow::play_normal_audio() {
    if (ui->sNAt->text() == "▶️") {
        ui->sNAt->setText("⏹");
        call_prayer("none", "Normal", 1);
    } else if (ui->sNAt->text() == "⏹") {
        ui->sNAt->setText("▶️");
        call_prayer("none", "Normal", 2);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

