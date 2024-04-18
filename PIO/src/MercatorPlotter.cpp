#include "MercatorPlotter.hpp"

MapPoint::MapPoint(float x, float y) {
    this->x = round(x);
    this->y = round(y);
}

MapPoint::MapPoint(int x, int y) {
    this->x = x;
    this->y = y;
}

MapPoint::MapPoint(const char* exceptionMessage) {
    this->hasError = true;
    this->exceptionMessage = exceptionMessage;
    this->x = -1;
    this->y = -1;
}

String MapPoint::toString() {
    if(this->hasError) {
        return String(this->exceptionMessage);
    }
    char buf[30];
    sprintf(buf, "x: %d, y: %d", this->x, this->y);
    return String(buf);
}

/*
 * MercatorPlotter
 * converts lat / lon to a point on a mercator map raster / image
 * map coordinate 0,0 are at upper left
 * map defaults to a width of 360 degrees with Null Island (0,0) at center
 * and no borders
 */
MercatorPlotter::MercatorPlotter(int imageWidth, int imageHeight, float degreeWidth) {
    this->imageWidth = imageWidth;
    this->imageHeight = imageHeight;
    this->degreeWidth = degreeWidth;
    this->botRightX = imageWidth;
    this->botRightY = imageHeight;
    MercatorPlotter::recalc();
}

/*
* set the top left corner of the map - account for image borders
*/
boolean MercatorPlotter::setTopLeft(int x, int y) {
    // validate x, y
    if(x >= 0 and x <= this->botRightX and
        y >= 0 and y <= this->botRightY) {
        this->topLeftX = x;
        this->topLeftY = y;
        MercatorPlotter::recalc();
        return true;
    } else {
        return false;
    }
}

/*
* set the bottom right corner of the map - account for image borders
*/
boolean MercatorPlotter::setBotRight(int x, int y) {
    // validate x, y
    if(x >= 0 and x <= this->imageWidth and
        y >= 0 and y <= this->imageHeight) {
        this->botRightX = x;
        this->botRightY = y;
        MercatorPlotter::recalc();
        return true;
    } else {
        return false;
    }
}

/*
* provide pixel offset for equator / prime meridian
*/
boolean MercatorPlotter::setNullIslandOffset(int latShift, int lonShift) {
    // validate latShift, lonShift
    if(abs(latShift) < (this->imageHeight/2) and
        abs(lonShift) < (this->map360width/2)) {
        this->latShift = latShift;
        this->lonShift = lonShift;
        MercatorPlotter::recalc();
        return true;
    } else {
        return false;
    }
}

MapPoint MercatorPlotter::plot(float lat, float lon) {
// test for valid lat / lon
if(lat >= -90 and lat <= 90 and
    lon >= -180 and lon <= 180) {

    // longitude: scale and shift
    float x = this->map360width * (180 + lon) / 360;
    x =  (x > this->map360width ?  x - this->map360width : x) + lonShift + this->topLeftX;

    // latitude: convert from degrees to radians
    float latR = lat * PI / 180;
    // compute the Mercator projection (w/ equator of 2pi units)
    float y = log(tan((latR/2) + (PI/4)));
    // scale and shift
    y = this->mapCenterY - (this->map360width * y / (2 * PI)) + this->latShift + this->topLeftY;

    // test for valid image coordinates
    if(x >= 0 and x <= this->imageWidth and
    y >= 0 and y <= this->imageHeight) {
    return MapPoint(x, y);
    } else {
    return MapPoint("Invalid image coordinates");
    }
} else {
    return MapPoint("Invalid lat / lon");
}

}

void MercatorPlotter::recalc() {
    this->mapWidth = this->botRightX - this->topLeftX;
    this->mapHeight = this->botRightY - this->topLeftY;
    this->mapCenterX = this->mapWidth / 2;
    this->mapCenterY = this->mapHeight / 2;
    this->map360width = this->mapWidth * (360 / this->degreeWidth);
}

