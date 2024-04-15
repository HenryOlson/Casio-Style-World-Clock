#ifndef MERCATOR_PLOTTER_H
#define MERCATOR_PLOTTER_H

#include <Arduino.h>

class MapPoint {
  public:
  int x;
  int y;
  boolean hasError = false;
  const char* exceptionMessage = "No Exception";

  MapPoint(float x, float y);
  MapPoint(int x, int y);
  MapPoint(const char* exceptionMessage);
  String toString();
};

/*
 * MercatorPlotter
 * converts lat / lon to a point on a mercator map raster / image
 * map defaults to 360 degrees width with Null Island (0,0) at center
 * and no borders with map coordinate 0,0 at upper left
 */
class MercatorPlotter {

  /* constructor parameters */
  int imageWidth;
  int imageHeight;
  float degreeWidth = 360;

  /* optional parameters */
  int topLeftX = 0;
  int topLeftY = 0;
  int botRightX;
  int botRightY;
  int latShift;
  int lonShift;

  /* computed values */
  float mapWidth;
  float mapHeight;
  float mapCenterX;
  float mapCenterY;
  float map360width;

  public:

  MercatorPlotter(int imageWidth, int imageHeight, float degreeWidth = 360);
  boolean setTopLeft(int x, int y);
  boolean setBotRight(int x, int y);
  boolean setNullIslandOffset(int latShift, int lonShift);
  MapPoint plot(float lat, float lon);

  private:

  void recalc();

};

#endif