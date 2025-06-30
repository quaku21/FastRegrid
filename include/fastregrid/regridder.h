/*
 * regridder.h
 * Core regridding functionality for geospatial data processing in FastRegrid.
 *
 * Author: Kevin Takyi Yeboah
 * Created: June, 2025
 *
 * Copyright (c) 2025 Kevin Takyi Yeboah
 * License: [MIT License, see LICENSE file]
 */

#ifndef FASTREGRID_REGRIDDER_H
#define FASTREGRID_REGRIDDER_H

#include "config.h"
#include "types.h"
#include "io.h"
#include "spatial_index.h"
#include "interpolation.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

namespace fastregrid
{

    class Regridder
    {
    public:
        // Constructor initializes with source and target file paths and config
        Regridder(const std::string &source_file, const std::string &target_file, const RegridConfig &config)
            : source_file_(source_file), target_file_(target_file), config_(config)
        {
            if (source_file_.empty() || target_file_.empty())
            {
                throw std::runtime_error("Source or target file path is empty");
            }
        }

        // Executes the regridding pipeline
        void regrid() const
        {
            // Step 1: Read source and target data
            InputReader source_reader(source_file_, config_);
            InputReader target_reader(target_file_, config_);

            if (config_.verbose)
            {
                std::cout << "Reading source data from: " << source_file_ << std::endl;
                std::cout << "Reading target data from: " << target_file_ << std::endl;
            }

            std::vector<SpatialData> source_points = source_reader.read_grid();
            std::vector<SpatialData> target_points = target_reader.read_grid();
            std::vector<std::string> headers = source_reader.read_headers();

            // Validate headers
            std::vector<std::string> target_headers = target_reader.read_headers();
            if (headers.size() < 3 || target_headers.size() < 3)
            {
                throw std::runtime_error("Invalid headers in source or target file");
            }
            if (config_.data_layout == GRID_BY_TIME && headers.size() != 15)
            {
                throw std::runtime_error("GRID_BY_TIME requires 12 monthly value columns plus Lon, Lat, Year");
            }
            if (headers.size() != target_headers.size())
            {
                throw std::runtime_error("Source and target files have different number of columns");
            }

            // Write gridlists if verbose
            source_reader.write_gridlist("source_gridlist.txt");
            target_reader.write_gridlist("target_gridlist.txt");

            // Step 2: Compute spatial mappings
            if (config_.verbose)
            {
                std::cout << "Computing spatial mappings..." << std::endl;
            }
            SpatialIndex index(source_points, config_);
            std::vector<std::tuple<double, double, double, double, double, size_t>> nn_mappings;
            std::vector<std::tuple<double, double, std::vector<std::tuple<double, double, double>>, size_t, bool>> idw_mappings;

            if (config_.interp_method == NEAREST_NEIGHBOR || config_.write_mappings)
            {
                nn_mappings = index.find_nearest_neighbors(target_points);
            }
            if (config_.interp_method == INVERSE_DISTANCE_WEIGHTED || config_.write_mappings)
            {
                idw_mappings = index.find_idw_neighbors(target_points);
            }

            // Step 3: Interpolate values
            if (config_.verbose)
            {
                std::cout << "Interpolating values..." << std::endl;
            }
            Interpolator interpolator(source_points, config_);
            std::vector<SpatialData> interpolated_points = interpolator.interpolate(target_points, nn_mappings, idw_mappings);

            // Step 4: Write outputs
            if (config_.verbose)
            {
                std::cout << "Writing outputs to: " << config_.output_path << std::endl;
            }
            OutputWriter writer(config_);
            if (config_.write_mappings)
            {
                if (!nn_mappings.empty())
                {
                    writer.write_nn_mappings(nn_mappings);
                }
                if (!idw_mappings.empty())
                {
                    writer.write_idw_mappings(idw_mappings);
                }
            }
            writer.write_regridded_data(interpolated_points, "regridded.txt", headers);

            if (config_.verbose)
            {
                std::cout << "Regridding completed successfully." << std::endl;
            }
        }

    private:
        std::string source_file_;
        std::string target_file_;
        const RegridConfig &config_;
    };

} // namespace fastregrid

#endif // FASTREGRID_REGRIDDER_H