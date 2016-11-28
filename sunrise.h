#ifndef METRIKA_SUNRISE_SUNRISE_H
#define METRIKA_SUNRISE_SUNRISE_H

#ifdef __cplusplus
extern "C" {
#endif

/* date of sunrise/sunset */
typedef struct
{
  int day;
  int month;
  int year;
} datetime_t;

/* location for sunrise/sunset */
typedef struct
{
  double lat; /* latitude, rad */
  double lon; /* longitude, rad */
} coord_t;

/* Sun's zenith for sunrise/sunset */
typedef enum
{
  official,    /* 90 degrees 50' */
  civil,       /* 96 degrees */
  nautical,    /* 102 degrees */
  astronomical /* 108 degrees */
} zenith_t;

/* time of sunrise in decimal hours [0..24) UTC
 *
 * return -1.0 if the sun never rises on this location (on the specified date) - Polar Night
 */
double sunrise(const datetime_t datetime,
               const coord_t coord,
               zenith_t zenith);

/* time of sunset in decimal hours [0..24) UTC
 *
 * return -1.0 if the sun never sets on this location (on the specified date) - Polar Day
 */
double sunset(const datetime_t datetime,
              const coord_t coord,
              zenith_t zenith);

#ifdef __cplusplus
}
#endif

#endif /* METRIKA_SUNRISE_SUNRISE_H */
