#include "../fastregrid/regridder.h"
#include <iostream>
#include <stdexcept>

using namespace fastregrid;

int main()
{
    // Configure RegridConfig with hardcoded settings
    RegridConfig config;
    config.output_path = "output/";                   // Output directory
    config.interp_method = INVERSE_DISTANCE_WEIGHTED; // Options: NEAREST_NEIGHBOR, INVERSE_DISTANCE_WEIGHTED
    config.data_layout = GRID_BY_TIME;                // Options: YEAR_BY_YEAR, GRID_BY_TIME
    config.radius = 100.0;                            // Search radius in km for IDW
    config.power = 2.0;                               // Power parameter for IDW
    config.min_points = 2;                            // Minimum points for IDW
    config.max_points = 4;                            // Maximum points for IDW
    config.precision = 5;                             // Output precision (decimal places)
    config.verbose = true;                            // Enable verbose logging
    config.write_mappings = true;                     // Write nn_mappings.txt and idw_mappings.txt
    config.adjust_longitude = false;                  // Adjust longitude to [-180, 180]

    // Hardcode input file paths
    std::string source_file = "source.txt";
    std::string target_file = "target.txt";

    system("pause");

    // Run regridding
    try
    {
        if (config.verbose)
        {
            std::cout << "Starting FastRegrid example with source: " << source_file
                      << ", target: " << target_file
                      << ", output: " << config.output_path << std::endl;
        }
        Regridder regridder(source_file, target_file, config);
        regridder.regrid();
        std::cout << "Regridding completed successfully. Outputs written to: " << config.output_path << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}