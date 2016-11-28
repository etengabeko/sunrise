#include "sunrise.h"

#include <math.h>

static double deg2rad(double deg)
{
  return (deg * M_PI / 180.0);
}

static double rad2deg(double rad)
{
  return (rad * 180.0 / M_PI);
}

static const double official_zenith_angle     = 90.0 + (50.0/60.0); /* 90 deg 50' */
static const double civil_zenith_angle        = 96.0;
static const double nautical_zenith_angle     = 102.0;
static const double astronomical_zenith_angle = 108.0;

static double cos_zenith(zenith_t zenith)
{
  double angle = 0.0;
  switch (zenith) {
    case official:
        angle = official_zenith_angle;
      break;
    case civil:
        angle = civil_zenith_angle;
      break;
    case nautical:
        angle = nautical_zenith_angle;
      break;
    case astronomical:
        angle = astronomical_zenith_angle;
      break;
  }
  return cos(deg2rad(angle));
}

static int day_of_year(const datetime_t* datetime)
{
  int n1 = floor(275 * datetime->month/9);
  int n2 = floor((datetime->month + 9)/12);
  int n3 = (1 + floor(datetime->year - 4*floor(datetime->year/4) + 2)/3);
  return (n1 - (n2*n3) + datetime->day - 30);
}

static double longitude_hour(const double lon)
{
  return rad2deg(lon)/15.0;
}

static double approx_rising_time(const int doy, const double lon)
{
  return (doy + ((6.0 - longitude_hour(lon))/24.0));
}

static double approx_setting_time(const int doy, const double lon)
{
  return (doy + ((18.0 - longitude_hour(lon))/24.0));
}

static double sun_mean_anomaly(const double approxtime)
{
  return (0.9856 * approxtime) - 3.289;
}

static double sun_true_longitude(const double approxtime)
{
  double sma = sun_mean_anomaly(approxtime);
  double L = sma + (1.916 * sin(deg2rad(sma)))
                 + (0.020 * sin(deg2rad(2*sma)))
                 + 282.634;
  while (L < 0.0) {
    L += 360.0;
  }
  while (L >= 360.0) {
    L -= 360.0;
  }
  return L;
}

static double sun_right_ascension(const double suntruelon)
{
  double RA = rad2deg(atan(0.91764 * tan(deg2rad(suntruelon))));

  int Lquadrant  = 90 * floor(suntruelon/90.0);
  int RAquadrant = 90 * floor(RA/90.0);

  return (RA + (Lquadrant - RAquadrant));
}

static double right_ascension_in_hours(const double ra)
{
  return ra/15.0;
}

static void sun_declination(const double suntruelon,
                            double* sin_decl,
                            double* cos_decl)
{
  *sin_decl = 0.39782 * sin(deg2rad(suntruelon));
  *cos_decl = cos(asin(*sin_decl));
}

static double local_mean_time(const double slhour,
                              const double rahour,
                              const double approxtime)
{
  return (  slhour + rahour
          - (0.06571 * approxtime)
          - 6.622);
}

extern double sunrise(const datetime_t datetime,
                      const coord_t coord,
                      zenith_t zenith)
{
  double atime = approx_rising_time(day_of_year(&datetime), coord.lon);
  double stlon = sun_true_longitude(atime);

  double sin_decl = 0.0;
  double cos_decl = 0.0;
  sun_declination(stlon, &sin_decl, &cos_decl);

  double cosH =  (cos_zenith(zenith) - sin_decl*sin(coord.lat))
               / (cos_decl*cos(coord.lat));
  if (cosH > 1.0) {
    return -1.0; /* sun never rises */
  }
  else if (cosH < -1.0) {
    return 0.0;
  }

  double H  = (360.0 - rad2deg(acos(cosH)))/15.0;
  double RA = right_ascension_in_hours(sun_right_ascension(stlon));
  double T  = local_mean_time(H, RA, atime);
  double UT = T - longitude_hour(coord.lon);

  while (UT < 0.0) {
    UT += 24.0;
  }
  while (UT >= 24.0) {
    UT -= 24.0;
  }

  return UT;
}

extern double sunset(const datetime_t datetime,
                     const coord_t coord,
                     zenith_t zenith)
{
  double atime = approx_setting_time(day_of_year(&datetime), coord.lon);
  double stlon = sun_true_longitude(atime);

  double sin_decl = 0.0;
  double cos_decl = 0.0;
  sun_declination(stlon, &sin_decl, &cos_decl);

  double cosH =  (cos_zenith(zenith) - sin_decl*sin(coord.lat))
               / (cos_decl*cos(coord.lat));
  if (cosH < -1.0) {
    return -1.0; /* sun never sets */
  }
  else if (cosH > 1.0) {
    return 0.0;
  }

  double H  = rad2deg(acos(cosH))/15.0;
  double RA = right_ascension_in_hours(sun_right_ascension(stlon));
  double T  = local_mean_time(H, RA, atime);
  double UT = T - longitude_hour(coord.lon);

  while (UT < 0.0) {
    UT += 24.0;
  }
  while (UT >= 24.0) {
    UT -= 24.0;
  }

  return UT;
}
