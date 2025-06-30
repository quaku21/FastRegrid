/*
 * interpolation.h
 * Provides interpolation methods (Nearest Neighbor, Inverse Distance Weighting) for FastRegrid.
 *
 * Author: Kevin Takyi Yeboah
 * Created: June, 2025
 *
 * Copyright (c) 2025 Kevin Takyi Yeboah
 * License: [MIT License, see LICENSE file]
 */

#ifndef FASTREGRID_INTERPOLATION_H
#define FASTREGRID_INTERPOLATION_H

#include "config.h"
#include "types.h"
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cmath>

namespace fastregrid
{

    class Interpolator
    {
    public:
        explicit Interpolator(const std::vector<SpatialData> &source_points, const RegridConfig &config)
            : source_points_(source_points), config_(config)
        {
            if (source_points_.empty())
            {
                throw std::runtime_error("Source point list is empty");
            }
            // Validate value sizes
            size_t value_size = source_points_[0].values.size();
            for (const auto &point : source_points_)
            {
                if (point.values.size() != value_size)
                {
                    throw std::runtime_error("Inconsistent value sizes in source points");
                }
            }
        }

        // Main interpolation function
        std::vector<SpatialData> interpolate(
            const std::vector<SpatialData> &target_points,
            const std::vector<std::tuple<double, double, double, double, double, size_t>> &nn_mappings,
            const std::vector<std::tuple<double, double, std::vector<std::tuple<double, double, double>>, size_t, bool>> &idw_mappings) const
        {
            if (config_.interp_method == NEAREST_NEIGHBOR)
            {
                return interpolate_nearest_neighbor(target_points, nn_mappings);
            }
            else if (config_.interp_method == INVERSE_DISTANCE_WEIGHTED)
            {
                return interpolate_idw(target_points, idw_mappings);
            }
            else
            {
                throw std::runtime_error("Unknown interpolation method");
            }
        }

    private:
        // Finds source point and copies its values to the interpolated point
        bool find_and_copy_source_values(
            const SpatialData &target,
            double source_lon,
            double source_lat,
            SpatialData &interpolated) const
        {
            for (const auto &src : source_points_)
            {
                if (std::abs(src.gridPoint.longitude - source_lon) < 1e-6 &&
                    std::abs(src.gridPoint.latitude - source_lat) < 1e-6 &&
                    src.time_step == target.time_step)
                {
                    interpolated.values = src.values;
                    return true;
                }
            }
            if (config_.verbose)
            {
                std::cerr << "Warning: No source point found for target ("
                          << target.gridPoint.longitude << ", " << target.gridPoint.latitude << ", " << target.time_step
                          << ") at source (" << source_lon << ", " << source_lat << ")" << std::endl;
            }
            return false;
        }

        // Nearest Neighbor interpolation
        std::vector<SpatialData> interpolate_nearest_neighbor(
            const std::vector<SpatialData> &target_points,
            const std::vector<std::tuple<double, double, double, double, double, size_t>> &mappings) const
        {
            std::vector<SpatialData> result;
            result.reserve(target_points.size());

            for (const auto &mapping : mappings)
            {
                auto [target_lon, target_lat, source_lon, source_lat, distance, target_idx] = mapping;
                if (target_idx >= target_points.size())
                {
                    throw std::runtime_error("Invalid target index in NN mapping");
                }
                const auto &target = target_points[target_idx];

                SpatialData interpolated = target;
                if (find_and_copy_source_values(target, source_lon, source_lat, interpolated))
                {
                    result.push_back(interpolated);
                }
            }

            if (result.empty())
            {
                throw std::runtime_error("No points interpolated in NN mode");
            }
            return result;
        }

        // IDW interpolation with fallback to Nearest Neighbor
        std::vector<SpatialData> interpolate_idw(
            const std::vector<SpatialData> &target_points,
            const std::vector<std::tuple<double, double, std::vector<std::tuple<double, double, double>>, size_t, bool>> &mappings) const
        {
            std::vector<SpatialData> result;
            result.reserve(target_points.size());

            for (const auto &mapping : mappings)
            {
                auto [target_lon, target_lat, sources, target_idx, is_fallback] = mapping;
                if (target_idx >= target_points.size())
                {
                    throw std::runtime_error("Invalid target index in IDW mapping");
                }
                const auto &target = target_points[target_idx];

                SpatialData interpolated = target;
                interpolated.values.clear();

                if (is_fallback)
                {
                    // Nearest Neighbor fallback (single source)
                    if (sources.size() != 1)
                    {
                        throw std::runtime_error("Invalid fallback mapping: expected one source point");
                    }
                    auto [source_lon, source_lat, distance] = sources[0];
                    if (find_and_copy_source_values(target, source_lon, source_lat, interpolated))
                    {
                        result.push_back(interpolated);
                    }
                }
                else
                {
                    // IDW interpolation
                    std::vector<double> weights;
                    std::vector<const SpatialData *> source_points;
                    weights.reserve(sources.size());
                    source_points.reserve(sources.size());

                    // Collect source points and weights
                    for (const auto &[source_lon, source_lat, distance] : sources)
                    {
                        const SpatialData *source = nullptr;
                        for (const auto &src : source_points_)
                        {
                            if (std::abs(src.gridPoint.longitude - source_lon) < 1e-6 &&
                                std::abs(src.gridPoint.latitude - source_lat) < 1e-6 &&
                                src.time_step == target.time_step)
                            {
                                source = &src;
                                break;
                            }
                        }
                        if (!source)
                        {
                            if (config_.verbose)
                            {
                                std::cerr << "Warning: No source point found for ("
                                          << source_lon << ", " << source_lat << ", " << target.time_step
                                          << ") in IDW interpolation" << std::endl;
                            }
                            continue;
                        }
                        double weight = distance > 1e-6 ? 1.0 / std::pow(distance, config_.power) : 1e6; // Avoid division by zero
                        weights.push_back(weight);
                        source_points.push_back(source);
                    }

                    if (source_points.empty())
                    {
                        if (config_.verbose)
                        {
                            std::cerr << "Warning: No valid source points for target ("
                                      << target_lon << ", " << target_lat << ", " << target.time_step
                                      << ") in IDW interpolation" << std::endl;
                        }
                        continue;
                    }

                    // Initialize interpolated values
                    interpolated.values.resize(source_points[0]->values.size(), 0.0);
                    double weight_sum = 0.0;
                    for (size_t i = 0; i < source_points.size(); ++i)
                    {
                        weight_sum += weights[i];
                        for (size_t j = 0; j < interpolated.values.size(); ++j)
                        {
                            interpolated.values[j] += weights[i] * source_points[i]->values[j];
                        }
                    }
                    for (auto &value : interpolated.values)
                    {
                        value /= weight_sum;
                    }
                    result.push_back(interpolated);
                }
            }

            if (result.empty())
            {
                throw std::runtime_error("No points interpolated in IDW mode");
            }
            return result;
        }

    private:
        const std::vector<SpatialData> &source_points_;
        const RegridConfig &config_;
    };

} // namespace fastregrid

#endif // FASTREGRID_INTERPOLATION_H