/*
 * types.h
 * Defines core data types and enumerations for geospatial data in FastRegrid.
 *
 * Author: Kevin Takyi Yeboah
 * Created: June, 2025
 *
 * Copyright (c) 2025 Kevin Takyi Yeboah
 * License: [MIT License, see LICENSE file]
 */

#ifndef FASTREGRID_TYPES_H
#define FASTREGRID_TYPES_H

#include <vector>
#include <string>

namespace fastregrid
{

    // Represents a geospatial point with longitude and latitude.
    struct GridPoint
    {
        double longitude; // Longitude in degrees
        double latitude;  // Latitude in degrees
    };

    // Represents data for a grid point at a specific time step.
    struct SpatialData
    {
        GridPoint gridPoint;        // Geospatial coordinates
        int time_step;              // Time identifier (e.g., year)
        std::vector<double> values; // Data values (e.g., 12 monthly values)
    };

    // Interpolation method options.
    enum InterpolationMethod
    {
        NEAREST_NEIGHBOR,
        INVERSE_DISTANCE_WEIGHTED
    };

    // Distance metric options.
    enum DistanceMetric
    {
        EUCLIDEAN,
        HAVERSINE
    };

    // Data layout options for input files.
    enum DataLayout
    {
        YEAR_BY_YEAR, // If file contains data for one year, all grid cells
        GRID_BY_TIME  // If file contains data for one grid cell, all years //Can contain all gridcells in one file
    };

} // namespace fastregrid

#endif // FASTREGRID_TYPES_H