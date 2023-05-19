#include "calc_prayer.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QMainWindow>
#include <ctime>
#include <cmath>
#include <cstring>

using namespace std;
unsigned int calc_method;
unsigned int asr_juristic;

double dhuhr_minutes;
const int NUM_ITERATIONS = 1;		// number of iterations needed to compute times
double julian_date;

static const char* TimeName[] = {"Fajr","Sunrise","Dhuhr","Asr","Sunset","Maghrib","Isha"};

enum JuristicMethod {Shafi, Hanafi};
enum TimeID{Fajr,Sunrise,Dhuhr,Asr,Sunset,Maghrib,Isha,TimesCount};
enum CalculationMethod
{
    MWL,    	// Muslim World League (MWL)
    Karachi, 	// University of Islamic Sciences, Karachi
    ISNA,   	// Islamic Society of North America (ISNA)
    Makkah, 	// Umm al-Qura, Makkah
    Egypt,  	// Egyptian General Authority of Survey
    Jafari,		// Jafari
    Tehran, 	// Tehran
    CalculationMethodsCount
};

enum AdjustingMethod // Adjusting Methods for Higher Latitudes
{
    None,      	// No adjustment
    MidNight,  	// middle of night
    OneSeventh,	// 1/7th of night
    AngleBased,	// angle/60th of night
};
AdjustingMethod adjust_high_lats;

struct MethodConfig
{

    double fajr_angle;
    bool   maghrib_is_minutes;
    double maghrib_value;		// angle or minutes
    bool   isha_is_minutes;
    double isha_value;		// angle or minutes

    MethodConfig()
    {
        fajr_angle = 18.0;
        maghrib_is_minutes = true;
        maghrib_value = 0.0;		// angle or minutes
        isha_is_minutes = false;
        isha_value = 18.0;		// angle or minutes
    }


    MethodConfig(double fajr_angle,
                 bool maghrib_is_minutes,
                 double maghrib_value,
                 bool isha_is_minutes,
                 double isha_value)
        : fajr_angle(fajr_angle)
        , maghrib_is_minutes(maghrib_is_minutes)
        , maghrib_value(maghrib_value)
        , isha_is_minutes(isha_is_minutes)
        , isha_value(isha_value)
    {
    }
};
MethodConfig method_params[CalculationMethodsCount];

typedef std::pair<double, double> DoublePair;
static std::string two_digits_format(int num);
static std::string int_to_string(int num);
static double fix_hour(double a);
static void get_float_time_parts(double time, int& hours, int& minutes);
static std::string float_time_to_time24(double time);
static std::string float_time_to_time12(double time, bool no_suffix = false);
double deg2rad(double d);
double rad2deg(double r);
static double dsin(double d);
static double dcos(double d);
static double dtan(double d);
static double darcsin(double x);
static double darccos(double x);
static double darctan(double x);
static double darctan2(double y, double x);
static double darccot(double x);
static double fix_angle(double a);
static double time_diff(double time1, double time2);
DoublePair sun_position(double jd);
double equation_of_time(double jd);
double sun_declination(double jd);
double compute_mid_day(double _t);
double compute_time(double g, double t);
void day_portion(double times[]);
double compute_asr(int step, double t);  // Shafii: step=1, Hanafi: step=2
void compute_times(double times[]);
double night_portion(double angle);
void adjust_high_lat_times(double times[]);
void adjust_times(double times[]);
void compute_day_times(double times[]);
double get_julian_date(int year, int month, int day);
void get_prayer_times(int year, int month, int day, double _latitude, double _longitude, double _timezo, double times[]);
void get_prayer_times(time_t date, double latitude, double longitude, double tzon, double times[]);

void MainWindow::calc_prayer(int cal_m, int madhab, double lati, double longit, double time_zone)
{
    char ch;
    adjust_high_lats 	= None;
    dhuhr_minutes 		= 0;
    latitude  			= lati;
    longitude 			= longit;
    Time_Zone           = time_zone;
    time_t date = time(NULL);

    calc_method = cal_m & 0x0F;
    method_params[0] = MethodConfig(18.5, true,  0.0, true,  90.0);	// Makkah
    method_params[1] = MethodConfig(19.5, true,  0.0, false, 17.5);	// Egypt
    method_params[2] = MethodConfig(18.0, true,  0.0, false, 18.0);	// Karachi
    method_params[3] = MethodConfig(15.0, true,  0.0, false, 15.0);	// ISNA
    method_params[4] = MethodConfig(18.0, true,  0.0, false, 17.0);	// MWL

    asr_juristic = madhab & 0x0F;

    double times[TimesCount];

    get_prayer_times(date, latitude, longitude, Time_Zone, times);

    fajr_n_24 = QString(float_time_to_time24(times[0]).c_str());
    shuruk_n_24 = QString(float_time_to_time24(times[1]).c_str());
    dhuhr_n_24 = QString(float_time_to_time24(times[2]).c_str());
    asr_n_24 = QString(float_time_to_time24(times[3]).c_str());
    maghrib_n_24 = QString(float_time_to_time24(times[5]).c_str());
    isha_n_24 = QString(float_time_to_time24(times[6]).c_str());

    if (clock_format == 1) {
        fajr_n = QString(float_time_to_time24(times[0]).c_str());
        shuruk_n = QString(float_time_to_time24(times[1]).c_str());
        dhuhr_n = QString(float_time_to_time24(times[2]).c_str());
        asr_n = QString(float_time_to_time24(times[3]).c_str());
        maghrib_n = QString(float_time_to_time24(times[5]).c_str());
        isha_n = QString(float_time_to_time24(times[6]).c_str());
    } else if (clock_format == 0) {
        if (sel_locale == "ar_FR") {
            fajr_n = QString(float_time_to_time12(times[0]).c_str());
            shuruk_n = QString(float_time_to_time12(times[1]).c_str());
            dhuhr_n = QString(float_time_to_time12(times[2]).c_str());
            asr_n = QString(float_time_to_time12(times[3]).c_str());
            maghrib_n = QString(float_time_to_time12(times[5]).c_str());
            isha_n = QString(float_time_to_time12(times[6]).c_str());
            fajr_n.replace(" AM", " ص");
            shuruk_n.replace(" AM", " ص");
            dhuhr_n.replace(" AM", " ص");
            dhuhr_n.replace(" PM", " م");
            asr_n.replace(" PM", " م");
            maghrib_n.replace(" PM", " م");
            isha_n.replace(" PM", " م");
        } else {
            fajr_n = QString(float_time_to_time12(times[0]).c_str());
            shuruk_n = QString(float_time_to_time12(times[1]).c_str());
            dhuhr_n = QString(float_time_to_time12(times[2]).c_str());
            asr_n = QString(float_time_to_time12(times[3]).c_str());
            maghrib_n = QString(float_time_to_time12(times[5]).c_str());
            isha_n = QString(float_time_to_time12(times[6]).c_str());
        }
    }
}

static std::string two_digits_format(int num)
{
    char tmp[16];
    tmp[0] = '\0';
    sprintf(tmp, "%2.2d", num);
    return std::string(tmp);
}
static std::string int_to_string(int num)
{
    char tmp[16];
    tmp[0] = '\0';
    if (num>=0 && num<=9) sprintf(tmp, "0%d", num);
    else sprintf(tmp, "%d", num);
    return std::string(tmp);
}
static double fix_hour(double a)
{
    a = a - 24.0 * floor(a / 24.0);
    a = a < 0.0 ? a + 24.0 : a;
    return a;
}

static void get_float_time_parts(double time, int& hours, int& minutes)
{
    time = fix_hour(time + 0.5 / 60);		// add 0.5 minutes to round
    hours = floor(time);
    minutes = floor((time - hours) * 60);
}
static std::string float_time_to_time24(double time)
{
    if(std::isnan(time))
        return std::string();
    int hours, minutes;
    get_float_time_parts(time, hours, minutes);
    return two_digits_format(hours) + ':' + two_digits_format(minutes);
}
static std::string float_time_to_time12(double time, bool no_suffix)
{
    if (isnan(time))
        return std::string();
    int hours, minutes;
    get_float_time_parts(time, hours, minutes);
    const char* suffix = hours >= 12 ? " PM" : " AM";
    hours = (hours + 12 - 1) % 12 + 1;
    return int_to_string(hours) + ':' + two_digits_format(minutes) + (no_suffix ? "" : suffix);
}

double deg2rad(double d)
{
    return d * M_PI / 180.0;
}

/* radian to degree */
double rad2deg(double r)
{
    return r * 180.0 / M_PI;
}
static double dsin(double d)
{
    return sin(deg2rad(d));
}

/* degree cos */
static double dcos(double d)
{
    return cos(deg2rad(d));
}

/* degree tan */
static double dtan(double d)
{
    return tan(deg2rad(d));
}

/* degree arcsin */
static double darcsin(double x)
{
    return rad2deg(asin(x));
}

/* degree arccos */
static double darccos(double x)
{
    return rad2deg(acos(x));
}

/* degree arctan */
static double darctan(double x)
{
    return rad2deg(atan(x));
}

/* degree arctan2 */
static double darctan2(double y, double x)
{
    return rad2deg(atan2(y, x));
}

/* degree arccot */
static double darccot(double x)
{
    return rad2deg(atan(1.0 / x));
}

/* range reduce angle in degrees. */
static double fix_angle(double a)
{
    a = a - 360.0 * floor(a / 360.0);
    a = a < 0.0 ? a + 360.0 : a;
    return a;
}

/* range reduce hours to 0..23 */

static double time_diff(double time1, double time2)
{
    return fix_hour(time2 - time1);
}


DoublePair sun_position(double jd)
{

    double d = jd - 2451545.0;	// jd is the given Julian date

    double g = fix_angle(357.529 + 0.98560028 * d);

    double q = fix_angle(280.459 + 0.98564736 * d);

    double L = fix_angle(q + 1.915 * dsin(g) + 0.020 * dsin(2 * g));

    double R = 1.00014 - 0.01671 * dcos(g) - 0.00014 * dcos(2 * g);
    double e = 23.439 - 0.00000036 * d;
    double RA = darctan2(dcos(e) * dsin(L), dcos(L)) / 15.0;
    double D = darcsin(dsin(e) * dsin(L)); 	// declination of the Sun

    RA = fix_hour(RA);

    double Eq_t = q / 15.0 - RA;			// equation of time

    return DoublePair(D, Eq_t);
}


double equation_of_time(double jd)
{
    return sun_position(jd).second;
}

double sun_declination(double jd)
{
    return sun_position(jd).first;
}
double compute_mid_day(double _t)
{
    double t = equation_of_time(julian_date + _t);
    double z = fix_hour(12 - t);
    return z;
}

double compute_time(double g, double t)
{
    double d = sun_declination(julian_date + t);
    double z = compute_mid_day(t);
    double v = 1.0 / 15.0 * darccos((-dsin(g) - dsin(d) * dsin(latitude)) / (dcos(d) * dcos(latitude)));
    return z + (g > 90.0 ? - v :  v);
}


void day_portion(double times[])
{
    for (int i = 0; i < TimesCount; ++i){
        times[i] /= 24.0;
    }
}

double compute_asr(int step, double t)  // Shafii: step=1, Hanafi: step=2
{
    double d = sun_declination(julian_date + t);
    double g = -darccot(step + dtan(fabs(latitude - d)));
    return compute_time(g, t);
}


void compute_times(double times[])
{
    day_portion(times);

    times[Fajr]    = compute_time(180.0 - method_params[calc_method].fajr_angle, times[Fajr]);

    times[Sunrise] = compute_time(180.0 - 0.833, times[Sunrise]);
    times[Dhuhr]   = compute_mid_day(times[Dhuhr]);
    times[Asr]     = compute_asr(1 + asr_juristic, times[Asr]);
    times[Sunset]  = compute_time(0.833, times[Sunset]);
    times[Maghrib] = compute_time(method_params[calc_method].maghrib_value, times[Maghrib]);
    times[Isha]    = compute_time(method_params[calc_method].isha_value, times[Isha]);
}
double night_portion(double angle)
{
    switch (adjust_high_lats)
    {
    case AngleBased:
        return angle / 60.0;
    case MidNight:
        return 1.0 / 2.0;
    case OneSeventh:
        return 1.0 / 7.0;
    default:
        // Just to return something!
        // In original library nothing was returned
        // Maybe I should throw an exception
        // It must be impossible to reach here
        return 0;
    }
}
void adjust_high_lat_times(double times[])
{
    double night_time = time_diff(times[Sunset], times[Sunrise]);		// sunset to sunrise

    // Adjust Fajr
    double fajr_diff = night_portion(method_params[calc_method].fajr_angle) * night_time;
    if (std::isnan(times[Fajr]) || time_diff(times[Fajr], times[Sunrise]) > fajr_diff)
        times[Fajr] = times[Sunrise] - fajr_diff;

    // Adjust Isha
    double isha_angle = method_params[calc_method].isha_is_minutes ? 18.0 : method_params[calc_method].isha_value;
    double isha_diff = night_portion(isha_angle) * night_time;
    if (std::isnan(times[Isha]) || time_diff(times[Sunset], times[Isha]) > isha_diff)
        times[Isha] = times[Sunset] + isha_diff;

    // Adjust Maghrib
    double maghrib_angle = method_params[calc_method].maghrib_is_minutes ? 4.0 : method_params[calc_method].maghrib_value;
    double maghrib_diff = night_portion(maghrib_angle) * night_time;
    if (std::isnan(times[Maghrib]) || time_diff(times[Sunset], times[Maghrib]) > maghrib_diff)
        times[Maghrib] = times[Sunset] + maghrib_diff;
}
void adjust_times(double times[])
{
    for (int i = 0; i < TimesCount; ++i)
        times[i] += Time_Zone - longitude / 15.0;

    times[Dhuhr] += dhuhr_minutes / 60.0;		// Dhuhr

    if (method_params[calc_method].maghrib_is_minutes)		// Maghrib
        times[Maghrib] = times[Sunset] + method_params[calc_method].maghrib_value / 60.0;

    times[Isha];

    if (method_params[calc_method].isha_is_minutes)		// Isha
        times[Isha] = times[Maghrib] + method_params[calc_method].isha_value / 60.0;

    if (adjust_high_lats != None)
        adjust_high_lat_times(times);
}

void compute_day_times(double times[])
{
    double default_times[] = { 5, 6, 12, 13, 18, 18, 18 };		// default times
    for (int i = 0; i < TimesCount; ++i)
        times[i] = default_times[i];

    for (int i = 0; i < NUM_ITERATIONS; ++i)
        compute_times(times);

    adjust_times(times);
}

double get_julian_date(int year, int month, int day)
{
    if (month <= 2)
    {
        year -= 1;
        month += 12;
    }

    double a = floor(year / 100.0);
    double b = 2 - a + floor(a / 4.0);
    double c = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day + b - 1524.5;

    return c;
}

void get_prayer_times(int year, int month, int day, double _latitude, double _longitude, double _timezo, double times[])
{
    latitude = _latitude;
    longitude = _longitude;
    Time_Zone = _timezo;
    julian_date = get_julian_date(year, month, day);
    julian_date = julian_date - longitude / (double) (15 * 24);
    compute_day_times(times);
}

void get_prayer_times(time_t date, double latitude, double longitude, double tzon, double times[])
{
    tm* t = localtime(&date);
    get_prayer_times((1900 + t->tm_year), (t->tm_mon + 1), (t->tm_mday), latitude, longitude, tzon, times);
}
