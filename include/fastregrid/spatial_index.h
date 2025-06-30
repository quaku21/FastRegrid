/*
 * spatial_index.h
 * Implements spatial indexing for efficient nearest neighbor searches in FastRegrid.
 *
 * Author: Kevin Takyi Yeboah
 * Created: June, 2025
 *
 * Copyright (c) 2025 Kevin Takyi Yeboah
 * License: [MIT License, see LICENSE file]
 */

#ifndef FASTREGRID_SPATIAL_INDEX_H
#define FASTREGRID_SPATIAL_INDEX_H

#include "config.h"
#include "types.h"
#include "utils.h"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace fastregrid
{

    class SpatialIndex
    {
    public:
        explicit SpatialIndex(const std::vector<SpatialData> &source_points, const RegridConfig &config)
            : source_points_(source_points), config_(config)
        {
            if (source_points_.empty())
            {
                throw std::runtime_error("Source point list is empty");
            }
        }

        // Finds nearest neighbor for each target point.
        std::vector<std::tuple<double, double, double, double, double, size_t>> find_nearest_neighbors(
            const std::vector<SpatialData> &target_points) const
        {
            std::vector<std::tuple<double, double, double, double, double, size_t>> mappings;
            mappings.reserve(target_points.size());

            for (size_t t_idx = 0; t_idx < target_points.size(); ++t_idx)
            {
                const auto &target = target_points[t_idx];
                double min_distance = std::numeric_limits<double>::max();
                double source_lon = 0.0, source_lat = 0.0;

                for (const auto &source : source_points_)
                {
                    double distance = utils::compute_distance(
                        target.gridPoint.longitude, target.gridPoint.latitude,
                        source.gridPoint.longitude, source.gridPoint.latitude,
                        config_.distance_metric);
                    if (distance < min_distance)
                    {
                        min_distance = distance;
                        source_lon = source.gridPoint.longitude;
                        source_lat = source.gridPoint.latitude;
                    }
                }

                if (min_distance == std::numeric_limits<double>::max())
                {
                    throw std::runtime_error("No valid source points found for target (" +
                                             std::to_string(target.gridPoint.longitude) + ", " +
                                             std::to_string(target.gridPoint.latitude) + ")");
                }

                double dist_km = config_.distance_metric == HAVERSINE
                                     ? min_distance
                                     : min_distance * 111.32 * std::cos(utils::to_radians(target.gridPoint.latitude));

                if (config_.verbose && dist_km > config_.radius)
                {
                    std::cerr << "Warning: Nearest source point for target (" << target.gridPoint.longitude << ", " << target.gridPoint.latitude
                              << ") is at distance " << dist_km << " km, exceeding radius " << config_.radius << " km"
                              << std::endl;
                }

                mappings.emplace_back(target.gridPoint.longitude, target.gridPoint.latitude,
                                      source_lon, source_lat, dist_km, t_idx);
            }

            return mappings;
        }

        // Finds up to max_points neighbors within radius for IDW, with fallback to Nearest Neighbor.
        std::vector<std::tuple<double, double, std::vector<std::tuple<double, double, double>>, size_t, bool>>
        find_idw_neighbors(const std::vector<SpatialData> &target_points) const
        {
            std::vector<std::tuple<double, double, std::vector<std::tuple<double, double, double>>, size_t, bool>> mappings;
            mappings.reserve(target_points.size());

            for (size_t t_idx = 0; t_idx < target_points.size(); ++t_idx)
            {
                const auto &target = target_points[t_idx];
                std::vector<std::tuple<double, double, double>> neighbors; // (source_lon, source_lat, distance)

                // Compute distances to all source points
                for (const auto &source : source_points_)
                {
                    double distance = utils::compute_distance(
                        target.gridPoint.longitude, target.gridPoint.latitude,
                        source.gridPoint.longitude, source.gridPoint.latitude,
                        config_.distance_metric);
                    if (config_.distance_metric == EUCLIDEAN)
                    {
                        double radius_deg = utils::km_to_degrees(config_.radius, target.gridPoint.latitude);
                        if (distance <= radius_deg)
                        {
                            neighbors.emplace_back(source.gridPoint.longitude, source.gridPoint.latitude, distance);
                        }
                    }
                    else
                    {
                        if (distance <= config_.radius)
                        {
                            neighbors.emplace_back(source.gridPoint.longitude, source.gridPoint.latitude, distance);
                        }
                    }
                }

                bool is_fallback = false;
                if (neighbors.size() < static_cast<size_t>(config_.min_points))
                {
                    // Fallback to Nearest Neighbor
                    if (config_.verbose)
                    {
                        std::cerr << "Warning: Only " << neighbors.size() << " points found within radius "
                                  << config_.radius << " km for target (" << target.gridPoint.longitude << ", " << target.gridPoint.latitude
                                  << "); falling back to Nearest Neighbor (min_points = " << config_.min_points << ")"
                                  << std::endl;
                    }
                    is_fallback = true;
                    neighbors.clear();
                    double min_distance = std::numeric_limits<double>::max();
                    double source_lon = 0.0, source_lat = 0.0;

                    for (const auto &source : source_points_)
                    {
                        double distance = utils::compute_distance(
                            target.gridPoint.longitude, target.gridPoint.latitude,
                            source.gridPoint.longitude, source.gridPoint.latitude,
                            config_.distance_metric);
                        if (distance < min_distance)
                        {
                            min_distance = distance;
                            source_lon = source.gridPoint.longitude;
                            source_lat = source.gridPoint.latitude;
                        }
                    }

                    if (min_distance != std::numeric_limits<double>::max())
                    {
                        double dist_km = config_.distance_metric == HAVERSINE
                                             ? min_distance
                                             : min_distance * 111.32 * std::cos(utils::to_radians(target.gridPoint.latitude));
                        neighbors.emplace_back(source_lon, source_lat, dist_km);
                    }
                }
                else
                {
                    // Sort by distance and take up to max_points
                    std::sort(neighbors.begin(), neighbors.end(),
                              [](const auto &a, const auto &b)
                              {
                                  return std::get<2>(a) < std::get<2>(b);
                              });
                    if (neighbors.size() > static_cast<size_t>(config_.max_points))
                    {
                        neighbors.resize(config_.max_points);
                    }
                    // Convert Euclidean distances to km if needed
                    if (config_.distance_metric == EUCLIDEAN)
                    {
                        for (auto &neighbor : neighbors)
                        {
                            std::get<2>(neighbor) = std::get<2>(neighbor) * 111.32 * std::cos(utils::to_radians(target.gridPoint.latitude));
                        }
                    }
                }

                if (neighbors.empty())
                {
                    throw std::runtime_error("No valid source points found for target (" +
                                             std::to_string(target.gridPoint.longitude) + ", " +
                                             std::to_string(target.gridPoint.latitude) + ")");
                }

                mappings.emplace_back(target.gridPoint.longitude, target.gridPoint.latitude,
                                      std::move(neighbors), t_idx, is_fallback);
            }

            return mappings;
        }

    private:
        const std::vector<SpatialData> &source_points_;
        const RegridConfig &config_;
    };

} // namespace fastregrid

#endif // FASTREGRID_SPATIAL_INDEX_H