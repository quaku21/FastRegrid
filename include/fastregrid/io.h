/*
 * io.h
 * Handles input/output operations for reading and writing geospatial data in FastRegrid.
 *
 * Author: Kevin Takyi Yeboah
 * Created: June, 2025
 *
 * Copyright (c) 2025 Kevin Takyi Yeboah
 * License: [MIT License, see LICENSE file]
 */

#ifndef FASTREGRID_IO_H
#define FASTREGRID_IO_H

#include "config.h"
#include "types.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <stdexcept>
#include <string>
#include <iostream>
#include <algorithm>

#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#else
#include <sys/stat.h>
#include <sys/types.h>
#define mkdir(dir) mkdir(dir, 0755)
#endif

namespace fastregrid
{

    class InputReader
    {
    public:
        explicit InputReader(const std::string &filename, const RegridConfig &config)
            : filename_(filename), config_(config) {}

        // Reads headers from the input file.
        std::vector<std::string> read_headers() const
        {
            std::ifstream file(filename_);
            if (!file.is_open())
            {
                throw std::runtime_error("Cannot open input file: " + filename_);
            }

            std::string line;
            std::getline(file, line);
            std::istringstream iss(line);
            std::vector<std::string> headers;
            std::string header;
            while (iss >> header)
            {
                headers.push_back(header);
            }
            file.close();
            return headers;
        }

        // Reads source or target gridpoints into a vector of SpatialData.
        std::vector<SpatialData> read_grid() const
        {
            std::ifstream file(filename_);
            if (!file.is_open())
            {
                throw std::runtime_error("Cannot open input file: " + filename_);
            }

            std::vector<SpatialData> points;
            std::string line;
            std::getline(file, line); // Skip header

            size_t line_num = 1;
            while (std::getline(file, line))
            {
                ++line_num;
                std::istringstream iss(line);
                SpatialData point;
                if (!(iss >> point.gridPoint.longitude >> point.gridPoint.latitude >> point.time_step))
                {
                    if (config_.verbose)
                    {
                        std::cerr << "Warning: Skipping malformed line " << line_num << " in file: " << filename_ << std::endl;
                    }
                    continue;
                }
                if (std::abs(point.gridPoint.latitude) > 90.0 || std::abs(point.gridPoint.longitude) > 360.0)
                {
                    throw std::runtime_error("Invalid coordinates at line " + std::to_string(line_num) + " in file: " + filename_);
                }
                if (config_.adjust_longitude)
                {
                    point.gridPoint.longitude = utils::adjust_longitude(point.gridPoint.longitude);
                }

                if (config_.data_layout == GRID_BY_TIME)
                {
                    point.values.resize(12); // Expect 12 monthly values
                    for (size_t i = 0; i < 12; ++i)
                    {
                        if (!(iss >> point.values[i]))
                        {
                            throw std::runtime_error("Missing monthly values at line " + std::to_string(line_num) + " in file: " + filename_);
                        }
                    }
                }
                else if (config_.data_layout == YEAR_BY_YEAR)
                {
                    double value;
                    while (iss >> value)
                    {
                        point.values.push_back(value);
                    }
                    if (point.values.empty())
                    {
                        throw std::runtime_error("No values found at line " + std::to_string(line_num) + " in file: " + filename_);
                    }
                }
                else
                {
                    throw std::runtime_error("Unknown data layout");
                }
                points.push_back(point);
            }
            file.close();
            if (points.empty())
            {
                throw std::runtime_error("Empty input file: " + filename_);
            }
            return points;
        }

        // Writes unique coordinates to a gridlist file
        void write_gridlist(const std::string &output_filename) const
        {
            auto points = read_grid();
            std::vector<std::pair<double, double>> unique_points;
            for (const auto &point : points)
            {
                unique_points.emplace_back(point.gridPoint.longitude, point.gridPoint.latitude);
            }
            // Sort and remove duplicates
            std::sort(unique_points.begin(), unique_points.end());
            unique_points.erase(std::unique(unique_points.begin(), unique_points.end()), unique_points.end());

            std::ofstream file(config_.output_path + output_filename);
            if (!file.is_open())
            {
                std::cerr << "Warning: Cannot open gridlist file: " + config_.output_path + output_filename << std::endl;
                return;
            }
            file << "Lon\t Lat\n";
            for (const auto &[lon, lat] : unique_points)
            {
                file << std::fixed << std::setprecision(config_.precision)
                     << std::setw(10) << lon
                     << std::setw(10) << lat << '\n';
            }
            file.close();
        }

    private:
        std::string filename_; // check if we really this here? FIXME
        const RegridConfig &config_;
    };

    class OutputWriter
    {
    public:
        explicit OutputWriter(const RegridConfig &config) : config_(config)
        {
            output_path_ = config_.output_path.empty() ? "./" : config_.output_path;
            if (output_path_.back() != '/' && output_path_.back() != '\\')
            {
                output_path_ += '/';
            }
            if (!output_path_.empty() && output_path_ != "./")
            {
                if (mkdir(output_path_.c_str()) != 0 && errno != EEXIST)
                {
                    throw std::runtime_error("Cannot create output directory: " + output_path_);
                }
            }
        }

        // Writes regridded data with headers
        void write_regridded_data(const std::vector<SpatialData> &points,
                                  const std::string &filename,
                                  const std::vector<std::string> &headers) const
        {
            std::ofstream file(output_path_ + filename);
            if (!file.is_open())
            {
                throw std::runtime_error("Cannot open output file: " + output_path_ + filename);
            }

            // Write headers
            for (size_t i = 0; i < headers.size(); ++i)
            {
                file << std::setw(i < 3 ? 10 : 12) << headers[i];
            }
            file << '\n';

            if (config_.data_layout == GRID_BY_TIME)
            {
                for (const auto &point : points)
                {
                    file << std::fixed << std::setprecision(config_.precision)
                         << std::setw(10) << point.gridPoint.longitude
                         << std::setw(10) << point.gridPoint.latitude
                         << std::setw(10) << point.time_step;
                    for (const auto &value : point.values)
                    {
                        file << std::setw(12) << value;
                    }
                    file << '\n';
                }
            }
            else if (config_.data_layout == YEAR_BY_YEAR)
            {
                for (const auto &point : points)
                {
                    file << std::fixed << std::setprecision(config_.precision)
                         << std::setw(10) << point.gridPoint.longitude
                         << std::setw(10) << point.gridPoint.latitude
                         << std::setw(10) << point.time_step;
                    for (const auto &value : point.values)
                    {
                        file << std::setw(12) << value;
                    }
                    file << '\n';
                }
            }
            file.close();
        }

        // Writes Nearest Neighbor mappings
        void write_nn_mappings(const std::vector<std::tuple<double, double, double, double, double, size_t>> &mappings) const
        {
            if (!config_.write_mappings)
                return;
            std::ofstream file(output_path_ + config_.nn_mappings_file);
            if (!file.is_open())
            {
                throw std::runtime_error("Cannot open NN mappings file: " + output_path_ + config_.nn_mappings_file);
            }

            file << "Target_Lon Target_Lat Source_Lon Source_Lat Distance(km) Target_Index\n";
            file << std::string(68, '-') << '\n';
            for (const auto &[target_lon, target_lat, source_lon, source_lat, distance, target_idx] : mappings)
            {
                file << std::fixed << std::setprecision(config_.precision)
                     << std::setw(10) << target_lon
                     << std::setw(10) << target_lat
                     << std::setw(10) << source_lon
                     << std::setw(10) << source_lat
                     << std::setw(12) << distance
                     << std::setw(12) << target_idx << '\n';
                file << std::string(68, '-') << '\n';
            }
            file.close();
        }

        // Writes IDW mappings
        void write_idw_mappings(const std::vector<std::tuple<double, double, std::vector<std::tuple<double, double, double>>, size_t, bool>> &mappings) const
        {
            if (!config_.write_mappings)
                return;
            std::ofstream file(output_path_ + config_.idw_mappings_file);
            if (!file.is_open())
            {
                throw std::runtime_error("Cannot open IDW mappings file: " + output_path_ + config_.idw_mappings_file);
            }

            file << "Target_Lon Target_Lat Source_Lon Source_Lat Distance(km) Target_Index Fallback\n";
            file << std::string(80, '-') << '\n';
            for (const auto &[target_lon, target_lat, sources, target_idx, is_fallback] : mappings)
            {
                for (const auto &[source_lon, source_lat, distance] : sources)
                {
                    file << std::fixed << std::setprecision(config_.precision)
                         << std::setw(10) << target_lon
                         << std::setw(10) << target_lat
                         << std::setw(10) << source_lon
                         << std::setw(10) << source_lat
                         << std::setw(12) << distance
                         << std::setw(12) << target_idx
                         << std::setw(8) << (is_fallback ? "NN" : "") << '\n';
                }
                file << std::string(80, '-') << '\n';
            }
            file.close();
        }

    private:
        const RegridConfig &config_;
        std::string output_path_;
    };

} // namespace fastregrid

#endif // FASTREGRID_IO_H