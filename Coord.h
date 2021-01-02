/* Coord.h
 * 
 * Copyright (C) 2003 by David Knapp
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __COORD_H__
#define __COORD_H__

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <ctype.h>
#include <algorithm>

#define COORD_EPSILON   1.e-10

class CEllipsoid;
class CCartesianCoord;

// -------------------------------------------------------------------------
// CLASS:  CLatLon
/*! 
   \brief  Class encapsulating a latitude/longitude coordinate.

   \author fizzymagic
   \date   9/6/2003
*/
// -------------------------------------------------------------------------
class CLatLon
{
public:
   //! Default constructor.
   CLatLon() : m_Latitude(0.), m_Longitude(0.) {};
   //! Constructor.
   CLatLon(double dLat, double dLon) : m_Latitude(dLat), m_Longitude(dLon) {};
   CLatLon(const char *szCoordString) { ParseCoords(szCoordString); };
   CLatLon(std::string& strCoordString) { ParseCoords(strCoordString); };

public:
   double m_Latitude;   //!< Latitude (degrees)
   double m_Longitude;  //!< Longitude (degrees)

   static const CEllipsoid m_Ellipsoid;   //!< Reference ellipsoid (meters).
   static const double m_Radius;          //!< Earth radius in meters.
   static const double m_GeosyncRadius;   //!< Geosynchronous satellite radius.
   static const double m_Deg2Rad;         //!< Conversion factor.
   static const std::string m_strZoneLetters;   //!< UTM zones.

public:
   double SphericalDistance(CLatLon& P);
   double SphericalDistance(CLatLon& P, double *pForwardAzimuth, double *pReverseAzimuth);
   bool IsBetween(CLatLon& P1, CLatLon& P2);
   double SphericalDistance(CLatLon& P1, CLatLon& P2);

   double ApproxEllipsoidDistance(CLatLon& P);

   CLatLon SphericalProjection(double dAzimuth, double dDistance);
   double RhumbDistance(CLatLon& P);
   double RhumbDistance(CLatLon& P, double *pForwardAzimuth, double *pReverseAzimuth);
   CLatLon RhumbProjection(double dAzimuth, double dDistance);
   double VincentyDistance(CLatLon& P);
   double VincentyDistance(CLatLon& P, double *pForwardAzimuth, double *pReverseAzimuth);
   CLatLon VincentyProjection(double dAzimuth, double dDistance);

   CCartesianCoord ToCartesian(double dElevation = 0.);
   double FromCartesian(CCartesianCoord& C);
   CCartesianCoord ToSphericalCartesian(double dElevation = 0.);
   void FromSphericalCartesian(CCartesianCoord& C);

   CCartesianCoord SphericalCross(CLatLon& P);

   double GeoSatelliteAzEl(double dSatLongitude, double dAltitude, double *pAzimuth, double *pElevation);
   double GeoSatelliteAzElSpherical(double dSatLongitude, double dAltitude, double *pAzimuth, double *pElevation);

public:
   std::string LatToDDD(void);
   std::string LatToDMM(void);
   std::string LatToDMS(void);
   std::string LongToDDD(void);
   std::string LongToDMM(void);
   std::string LongToDMS(void);

   std::string ToDDD(void);
   std::string ToDMM(void);
   std::string ToDMS(void);
   std::string ToUTM(void);

   bool ParseCoords(const char *szCoordString);
   bool ParseCoords(std::string& strCoordString) {
      return ParseCoords(strCoordString.c_str());
   };
   bool ParseCoords(const char *szLatString, const char *szLongString);
   bool ParseCoords(std::string& strLatString, std::string& strLongString) {
      return ParseCoords(strLatString.c_str(), strLongString.c_str());
   };
   bool ParseDegreeString(const char *szDegrees, double *dResult);
   bool ParseDegreeString(std::string& strDegrees, double *dResult) {
      return ParseDegreeString(strDegrees.c_str(), dResult);
   }
   bool ParseUTM(const char *szCoordString);
   bool ParseUTM(std::string& strCoordString) {
      return ParseUTM(strCoordString.c_str());
   };

   static void CleanCoordString(std::string& strCoordString);
   static bool SplitCoordString(std::string& strCoordString, std::string& strLatString, std::string& strLongString);
   static double NormalizeLatitude(double dLatitude);
   static double NormalizeLongitude(double dLongitude);

protected:
   int GetZone(void);
   char GetZoneLetter(void);
   bool ConvertUTM(int iZone, char cZoneLetter, double dEasting, double dNorthing);
};

// -------------------------------------------------------------------------
// CLASS:  CEllipsoid
/*! 
   \brief  Class encapsulating ellipsoidal parameters for the Earth

   \author fizzymagic
   \date   9/6/2003

   All parameters should be in meters.
*/
// -------------------------------------------------------------------------
class CEllipsoid
{
public:
   //! Default constructor.  Uses WGS84 ellipsoid.
   CEllipsoid() : m_a(6378137.00), m_fInv(298.257223563) {};
   //! Constructor.
   CEllipsoid(double a, double fInv) : m_a(a), m_fInv(fInv) {};

public:
   double m_a;       //!< Semimajor axis (meters)
   double m_fInv;    //!< 1/flatness

public:
   double GetC(void) { return m_a * (m_fInv - 1.) / m_fInv; };
};

// -------------------------------------------------------------------------
// CLASS:  CCartesianCoord
/*! 
   \brief  Class encapsulating xyz Cartesian coordinates.

   \author fizzymagic
   \date   9/6/2003
*/
// -------------------------------------------------------------------------
class CCartesianCoord
{
public:
   //! Default constructor.
   CCartesianCoord() : m_x(0.), m_y(0.), m_z(0.) {};
   //! Constructor from individual x, y, and z values.
   CCartesianCoord(double x, double y, double z) : m_x(x), m_y(y), m_z(z) {};
   //! Constructor from a CLatLon coordinate.  Uses the ellipsoid.
   CCartesianCoord(CLatLon& C) {
      *this = C.ToCartesian();
   };
   CCartesianCoord(CLatLon& C, double dElevation) {
      *this = C.ToCartesian(dElevation);
   };

public:
   double m_x;    //!< X coordinate.
   double m_y;    //!< Y coordinate.
   double m_z;    //!< Z coordinate.

public:
   //! Cartesian dot product.
   double dot(CCartesianCoord& C) {
      return m_x * C.m_x + m_y * C.m_y + m_z * C.m_z;
   };

   //! Cartesian cross product.
   CCartesianCoord cross(CCartesianCoord& C) {
      return CCartesianCoord(m_y * C.m_z - m_z * C.m_y, 
                             m_z * C.m_x - m_x * C.m_z, 
                             m_x * C.m_y - m_y * C.m_x);
   };

   //! Cartesian scaling.
   CCartesianCoord operator*(double R) {
      return CCartesianCoord(R * m_x, R * m_y, R * m_z);
   };
   CCartesianCoord& operator*=(double R) {
      m_x *= R; m_y *= R; m_z *= R;
      return *this;
   };
   CCartesianCoord operator/(double R) {
      return CCartesianCoord(m_x/R, m_y/R, m_z/R);
   };
   CCartesianCoord& operator/=(double R) {
      m_x /= R; m_y /= R; m_z /= R;
      return *this;
   };

   //! Addition of two Cartesian coordinates.
   CCartesianCoord operator+(CCartesianCoord& C) {
      return CCartesianCoord(m_x + C.m_x, m_y + C.m_y, m_z + C.m_z);
   };
   CCartesianCoord& operator+=(CCartesianCoord& C) {
      m_x += C.m_x; m_y += C.m_y; m_z += C.m_z;
      return *this;
   };

   //! Subtraction.
   CCartesianCoord operator-(CCartesianCoord& C) {
      return CCartesianCoord(m_x - C.m_x, m_y - C.m_y, m_z - C.m_z);
   };
   CCartesianCoord& operator-=(CCartesianCoord& C) {
      m_x -= C.m_x; m_y -= C.m_y; m_z -= C.m_z;
      return *this;
   };

   //! Negation.
   CCartesianCoord operator-(void) {
      return CCartesianCoord(-m_x, -m_y, -m_z);
   };


   //! Cartesian norm.
   double Norm(void) {
      return sqrt(m_x*m_x + m_y*m_y + m_z*m_z);
   };

   //! Cartesian normalize.
   CCartesianCoord& Normalize(void) {
      double dNorm = Norm();
      m_x /= dNorm; m_y /= dNorm; m_z /= dNorm;
      return *this;
   };
};

void findandreplace(std::string& strSource, const char *szFind, const char *szReplace);
void trim(std::string& strSource);

#endif   // __COORD_H__
