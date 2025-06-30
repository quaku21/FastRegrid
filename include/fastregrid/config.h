/*
 * config.h
 * Defines configuration structures and settings for the FastRegrid library.
 *
 * Author: Kevin Takyi Yeboah
 * Created: June, 2025
 *
 * Copyright (c) 2025 Kevin Takyi Yeboah
 * License: [MIT License, see LICENSE file]
 */

#ifndef FASTREGRID_CONFIG_H
#define FASTREGRID_CONFIG_H

#include "types.h"
#include <string>
#include <stdexcept>

namespace fastregrid
{

    // Configuration for regridding operations.
    struct RegridConfig
    {
        InterpolationMethod interp_method = INVERSE_DISTANCE_WEIGHTED; // Interpolation method
        DistanceMetric distance_metric = HAVERSINE;                    // Distance metric
        DataLayout data_layout = GRID_BY_TIME;                         // Input data layout
        double radius = 100.0;                                         // IDW radius in km (converted to degrees for Euclidean)
        double power = 2.0;                                            // IDW weighting power
        int max_points = 5;                                            // Desired number of source points for IDW
        int min_points = max_points;                                   // Minimum points for IDW, triggers Nearest Neighbor fallback
        bool adjust_longitude = true;                                  // Adjust longitude from 0-360 to -180-180
        int precision = 5;                                             // Output decimal precision
        bool verbose = false;                                          // Enable verbose logging
        bool write_mappings = false;                                   // Write mapping files
        std::string nn_mappings_file = "nn_mappings.txt";              // Nearest Neighbor mappings file
        std::string idw_mappings_file = "idw_mappings.txt";            // IDW mappings file
        size_t chunk_size = 1000;                                      // Max lines to process at once
        std::string output_path = "./";                                // Output directory for all files (relative or absolute)
    };

    // Builder class for constructing RegridConfig with validation.
    class RegridConfigBuilder
    {
    public:
        RegridConfigBuilder() = default;

        RegridConfigBuilder &set_interpolation(InterpolationMethod method)
        {
            config_.interp_method = method;
            return *this;
        }

        RegridConfigBuilder &set_distance_metric(DistanceMetric metric)
        {
            config_.distance_metric = metric;
            return *this;
        }

        RegridConfigBuilder &set_data_layout(DataLayout layout)
        {
            config_.data_layout = layout;
            return *this;
        }

        RegridConfigBuilder &set_radius(double radius)
        {
            if (radius < 0.0)
            {
                throw std::invalid_argument("Radius must be non-negative");
            }
            config_.radius = radius;
            return *this;
        }

        RegridConfigBuilder &set_power(double power)
        {
            if (power <= 0.0)
            {
                throw std::invalid_argument("Power must be positive");
            }
            config_.power = power;
            return *this;
        }

        RegridConfigBuilder &set_max_points(int max_points)
        {
            if (max_points <= 0)
            {
                throw std::invalid_argument("Max points must be positive");
            }
            config_.max_points = max_points;
            if (config_.min_points > max_points)
            {
                config_.min_points = max_points;
            }
            return *this;
        }

        RegridConfigBuilder &set_min_points(int min_points)
        {
            if (min_points <= 0)
            {
                throw std::invalid_argument("Min points must be positive");
            }
            if (min_points > config_.max_points)
            {
                throw std::invalid_argument("Min points cannot exceed max points");
            }
            config_.min_points = min_points;
            return *this;
        }

        RegridConfigBuilder &set_adjust_longitude(bool adjust)
        {
            config_.adjust_longitude = adjust;
            return *this;
        }

        RegridConfigBuilder &set_precision(int precision)
        {
            if (precision < 0)
            {
                throw std::invalid_argument("Precision must be non-negative");
            }
            config_.precision = precision;
            return *this;
        }

        RegridConfigBuilder &set_verbose(bool verbose)
        {
            config_.verbose = verbose;
            return *this;
        }

        RegridConfigBuilder &set_write_mappings(bool write)
        {
            config_.write_mappings = write;
            return *this;
        }

        RegridConfigBuilder &set_nn_mappings_file(const std::string &filename)
        {
            if (filename.empty())
            {
                throw std::invalid_argument("Nearest Neighbor mappings filename cannot be empty");
            }
            config_.nn_mappings_file = filename;
            return *this;
        }

        RegridConfigBuilder &set_idw_mappings_file(const std::string &filename)
        {
            if (filename.empty())
            {
                throw std::invalid_argument("IDW mappings filename cannot be empty");
            }
            config_.idw_mappings_file = filename;
            return *this;
        }

        RegridConfigBuilder &set_chunk_size(size_t chunk_size)
        {
            if (chunk_size == 0)
            {
                throw std::invalid_argument("Chunk size must be positive");
            }
            config_.chunk_size = chunk_size;
            return *this;
        }

        RegridConfigBuilder &set_output_path(const std::string &path)
        {
            config_.output_path = path;
            return *this;
        }

        RegridConfig build() const
        {
            return config_;
        }

    private:
        RegridConfig config_;
    };

} // namespace fastregrid

#endif // FASTREGRID_CONFIG_H