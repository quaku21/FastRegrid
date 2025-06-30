/*
 * utils.h
 * Provides utility functions for geospatial calculations in FastRegrid.
 *
 * Author: Kevin Takyi Yeboah
 * Created: June, 2025
 *
 * Copyright (c) 2025 Kevin Takyi Yeboah
 * License: [MIT License, see LICENSE file]
 */

#ifndef FASTREGRID_UTILS_H
#define FASTREGRID_UTILS_H
#define _USE_MATH_DEFINES

#include "types.h"
#include <cmath>
#include <math.h>
#include <stdexcept>

namespace fastregrid
{

    namespace utils
    {

        // Converts degrees to radians.
        inline double to_radians(double degrees)
        {
            return degrees * M_PI / 180.0;
        }

        // Normalizes longitude to [-180, 180] degrees.
        inline double adjust_longitude(double lon)
        {
            while (lon > 180.0)
            {
                lon -= 360.0;
            }
            while (lon < -180.0)
            {
                lon += 360.0;
            }
            return lon;
        }

        // Converts distance from km to degrees for Euclidean distance, adjusted for latitude.
        inline double km_to_degrees(double km, double latitude)
        {
            if (km < 0.0)
            {
                throw std::invalid_argument("Distance in km must be non-negative");
            }
            if (std::abs(latitude) > 90.0)
            {
                throw std::invalid_argument("Latitude must be in [-90, 90]");
            }
            // 111.32 km per degree of latitude, adjusted by cos(lat) for longitude.
            double cos_lat = std::cos(to_radians(latitude));
            if (std::abs(cos_lat) < 1e-10)
            {
                // Near poles, use a small value to avoid division by zero.
                cos_lat = 1e-10;
            }
            return km / (111.32 * cos_lat);
        }

        // Computes distance between two points using Haversine or Euclidean metric.
        // Returns distance in km (Haversine) or degrees (Euclidean, convert to km for output).
        double compute_distance(double lon1, double lat1, double lon2, double lat2,
                                DistanceMetric metric)
        {
            // Validate inputs.
            if (std::abs(lat1) > 90.0 || std::abs(lat2) > 90.0)
            {
                throw std::invalid_argument("Latitudes must be in [-90, 90]");
            }
            if (std::abs(lon1) > 360.0 || std::abs(lon2) > 360.0)
            {
                throw std::invalid_argument("Longitudes must be in [-360, 360]");
            }

            if (metric == HAVERSINE)
            {
                // Haversine formula for great-circle distance in km.
                constexpr double EARTH_RADIUS_KM = 6371.0;
                double lat1_rad = to_radians(lat1);
                double lat2_rad = to_radians(lat2);
                double delta_lat = to_radians(lat2 - lat1);
                double delta_lon = to_radians(lon2 - lon1);

                double a = std::sin(delta_lat / 2.0) * std::sin(delta_lat / 2.0) +
                           std::cos(lat1_rad) * std::cos(lat2_rad) *
                               std::sin(delta_lon / 2.0) * std::sin(delta_lon / 2.0);
                double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
                return EARTH_RADIUS_KM * c;
            }
            else if (metric == EUCLIDEAN)
            {
                // Euclidean distance in degrees (lon-lat plane).
                double delta_lon = lon2 - lon1;
                double delta_lat = lat2 - lat1;
                return std::sqrt(delta_lon * delta_lon + delta_lat * delta_lat);
            }
            else
            {
                throw std::invalid_argument("Unknown distance metric");
            }
        }

    } // namespace utils

} // namespace fastregrid

#endif // FASTREGRID_UTILS_H