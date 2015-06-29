#include <pebble.h>
#include "startline.h"

  double mysin(double x) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "in sin called with %d", (int)(x * 1000));
  while (x > M_PI)
    x = x - 2.0*M_PI;
  while (x < -M_PI)
    x = x + 2.0*M_PI;
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "in sin x=%d", (int)(x * 1000));
  return x - (x*x*x)/(3*2) + (x*x*x*x*x)/(5*4*3*2) - (x*x*x*x*x*x*x)/(7*6*5*4*3*2) + (x*x*x*x*x*x*x*x*x)/(9*8*7*6*5*4*3*2) - (x*x*x*x*x*x*x*x*x*x*x)/(11*10*9*8*7*6*5*4*3*2);
}



//cos(a) = sin( pi/2 - a)

double mycos(double x) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "in cos called with %d", (int)(x * 1000));
  return mysin(M_PI / 2 - x);
}