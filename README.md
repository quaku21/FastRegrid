# FastRegrid

**FastRegrid** is a C++ library for regridding geospatial data between source and target grids using Nearest Neighbor (NN) or Inverse Distance Weighting (IDW) interpolation, with a minimum requirement of C++17 for compilation. It supports two generic data layouts: `YEAR_BY_YEAR` and `GRID_BY_TIME` commonly used in climate simulators and ecological models. Designed for flexible geospatial data processing, it efficiently handles text-based grid files to produce regridded outputs with associated mapping files for transparency.

## Features

- **Interpolation Methods**:
  - Nearest Neighbor (NN): Assigns values from the closest source point within `radius`, warns and searches beyond `radius` if no source point is found within `radius`.
  - Inverse Distance Weighted (IDW): Computes weighted averages (inversely proportional to distance from target point) using up to `max_points` source points within `radius`, with fallback to NN if fewer than `min_points` are found.
- **Data Layouts**:
  - `YEAR_BY_YEAR`: For files containing data for one year with all grid cells.
  - `GRID_BY_TIME`: For files containing data for one grid cell with all years. May contain all gridcells for all years in one file.
- **Distance Metrics**: Euclidean or Haversine (great-circle) for geospatial calculations.
- **Configuration**: Flexible settings for radius, power, precision, and verbosity.
- **Outputs**: Regridded data (`regridded.txt`) and optional mappings (`nn_mappings.txt`, `idw_mappings.txt`).
- **Cross-Platform**: Supports Windows and Unix via standard C++ and platform-specific directory creation.
- **Dependency-Free**: Uses only the C++ standard library.

## Dependencies

- C++ compiler >=C++17 (e.g., GCC 5+, Clang 3.4+, MSVC 2015+).
- CMake 3.10+ for building.
- No external libraries other than standard C++ libraries.

## Installation

1. Clone or download the repository:
   ```bash
   git clone <repository-url>
   cd FastRegrid
   ```
2. Create a build directory:
   ```bash
   mkdir build && cd build
   ```
3. Run CMake and build:
   ```bash
   cmake ..
   cmake --build .
   ```
4. Outputs:
   - Library: `libfastregrid.a` (Unix) or `fastregrid.lib` (Windows) in `build/`.
   - Example executable: `bin/fastregrid_example`.
5. Optional: Install library and headers:
   ```bash
   cmake --install .
   ```
   - Installs to `lib/` and `include/fastregrid/`.

## Usage

1. **Include the Library**:

   - Link against `fastregrid` and include headers (e.g., `regridder.h`).
   - Example compilation (manual):
     ```bash
     g++ -std=c++17 -Ipath/to/FastRegrid -Lpath/to/build your_app.cpp -lfastregrid -o your_app
     ```

2. **Configure `RegridConfig`**:

   - Set options like `interp_method`, `data_layout`, and `output_path` (see [Configuration](#configuration)).
   - Example:
     ```cpp
     fastregrid::RegridConfig config;
     config.output_path = "output/";
     config.interp_method = fastregrid::INVERSE_DISTANCE_WEIGHTED;
     config.data_layout = fastregrid::GRID_BY_TIME;
     config.verbose = true;
     ```

3. **Use `Regridder`**:
   - Initialize with source/target file paths and `RegridConfig`.
   - Call `regrid()` to process data.
   - Example:
     ```cpp
     #include "regridder.h"
     int main() {
         fastregrid::RegridConfig config;
         config.output_path = "output/";
         config.interp_method = fastregrid::INVERSE_DISTANCE_WEIGHTED;
         config.data_layout = fastregrid::GRID_BY_TIME;
         config.verbose = true;
         fastregrid::Regridder regridder("source.txt", "target.txt", config);
         try {
             regridder.regrid();
         } catch (const std::exception& e) {
             std::cerr << "Error: " << e.what() << "\n";
             return 1;
         }
         return 0;
     }
     ```

## Configuration

`RegridConfig` (defined in `config.h`) controls regridding behavior:

| Option              | Type                  | Default                     | Description                                        |
| ------------------- | --------------------- | --------------------------- | -------------------------------------------------- |
| `output_path`       | `std::string`         | `"./"`                      | Directory for output files.                        |
| `interp_method`     | `InterpolationMethod` | `INVERSE_DISTANCE_WEIGHTED` | `NEAREST_NEIGHBOR` or `INVERSE_DISTANCE_WEIGHTED`. |
| `data_layout`       | `DataLayout`          | `GRID_BY_TIME`              | `YEAR_BY_YEAR` or `GRID_BY_TIME`.                  |
| `distance_metric`   | `DistanceMetric`      | `HAVERSINE`                 | `EUCLIDEAN` or `HAVERSINE`.                        |
| `radius`            | `double`              | `100.0`                     | Search radius in km for IDW.                       |
| `power`             | `double`              | `2.0`                       | IDW weight power (`w = 1/d^power`).                |
| `min_points`        | `int`                 | `2`                         | Minimum source points for IDW (else NN fallback).  |
| `max_points`        | `int`                 | `4`                         | Maximum source points for IDW.                     |
| `precision`         | `int`                 | `5`                         | Decimal places for output.                         |
| `verbose`           | `bool`                | `false`                     | Enable logging to `std::cerr`.                     |
| `write_mappings`    | `bool`                | `false`                     | Write `nn_mappings.txt` and `idw_mappings.txt`.    |
| `adjust_longitude`  | `bool`                | `false`                     | Adjust longitude to [-180, 180].                   |
| `nn_mappings_file`  | `std::string`         | `"nn_mappings.txt"`         | NN mappings output file name.                      |
| `idw_mappings_file` | `std::string`         | `"idw_mappings.txt"`        | IDW mappings output file name.                     |

## Input/Output Formats

### Input Files

- **Source File**:
  - Text file with headers and data.
  - `GRID_BY_TIME`: `Lon Lat Year Month1 ... Month12` (15 columns).
    ```text
    Lon Lat Year Month1 Month2 ... Month12
    87.25 46.25 2020 0.1 0.12 ... 0.22
    86.25 46.25 2020 0.09 0.11 ... 0.21
    ```
  - `YEAR_BY_YEAR`: `Lon Lat Year Value1 ... ValueN` (variable columns).
    ```text
    Lon Lat Year Flux_Fire Flux_Estab
    87.25 46.25 2020 0.5 1.2
    ```
- **Target File**:
  - Specifies desired grid points, typically `Lon Lat Year` (values optional).
    ```text
    Lon Lat Year
    88.0 46.0 2020
    ```

### Output Files

- **regridded.txt**:
  - Regridded data with same headers as source.
  - Example (`GRID_BY_TIME`):
    ```text
    Lon       Lat       Year  Month1  Month2  ... Month12
    88.00000  46.00000  2020  0.11000 0.13000 ... 0.14000
    ```
- **nn_mappings.txt** (if `write_mappings = true`):
  - Maps target points to nearest source point.
    ```text
    Target_Lon Target_Lat Source_Lon Source_Lat Distance(km) Target_Index
    --------------------------------------------------------------------
    88.00000   46.00000   87.25000   46.25000   76.89000     0
    --------------------------------------------------------------------
    ```
- **idw_mappings.txt** (if `write_mappings = true`):
  - Maps target points to multiple source points for IDW (or NN fallback).
    ```text
    Target_Lon Target_Lat Source_Lon Source_Lat Distance(km) Target_Index Fallback
    --------------------------------------------------------------------------------
    88.00000   46.00000   87.25000   46.25000   76.89000     0        false
    88.00000   46.00000   86.25000   46.25000   76.89000     0        false
    --------------------------------------------------------------------------------
    ```
- **source_gridlist.txt**, **target_gridlist.txt** (if `verbose = true`):
  - Unique coordinates for debugging.
    ```text
    Lon Lat
    87.25000 46.25000
    ```

## Example

The `examples/example.cpp` file demonstrates library usage with hardcoded settings:

```cpp
#include "regridder.h"
#include <iostream>
#include <stdexcept>

using namespace fastregrid;

int main() {
    // Configure RegridConfig
    RegridConfig config;
    config.output_path = "output/";
    config.interp_method = INVERSE_DISTANCE_WEIGHTED;
    config.data_layout = GRID_BY_TIME;
    config.radius = 100.0;
    config.power = 2.0;
    config.min_points = 2;
    config.max_points = 4;
    config.precision = 5;
    config.verbose = true;
    config.write_mappings = true;
    config.adjust_longitude = false;

    // Input files
    std::string source_file = "source.txt";
    std::string target_file = "target.txt";

    // Run regridding
    try {
        if (config.verbose) {
            std::cout << "Starting FastRegrid example with source: " << source_file
                      << ", target: " << target_file
                      << ", output: " << config.output_path << "\n";
        }
        Regridder regridder(source_file, target_file, config);
        regridder.regrid();
        std::cout << "Regridding completed successfully. Outputs written to: " << config.output_path << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
```

### Running the Example

1. Build the project (see [Installation](#installation)).
2. Ensure `source.txt` and `target.txt` exist in the working directory.
3. Run:
   ```bash
   ./bin/fastregrid_example
   ```
4. Outputs appear in `output/` (e.g., `regridded.txt`, `idw_mappings.txt`).

### Sample Input

- `source.txt` (`GRID_BY_TIME`):
  ```text
  Lon Lat Year Month1 Month2 Month3 Month4 Month5 Month6 Month7 Month8 Month9 Month10 Month11 Month12
  87.25 46.25 2020 0.1 0.12 0.13 0.14 0.15 0.16 0.17 0.18 0.19 0.2 0.21 0.22
  86.25 46.25 2020 0.09 0.11 0.12 0.13 0.14 0.15 0.16 0.17 0.18 0.19 0.2 0.21
  ```
- `target.txt`:
  ```text
  Lon Lat Year
  88.0 46.0 2020
  ```

## Directory Structure

- `config.h`: Defines `RegridConfig` for settings.
- `types.h`: Defines `SpatialData`, `GridPoint`, enums (`InterpolationMethod`, `DataLayout`, `DistanceMetric`).
- `utils.h`: Utility functions (e.g., `compute_distance`, `adjust_longitude`).
- `io.h`: `InputReader` and `OutputWriter` for file I/O.
- `spatial_index.h`: `SpatialIndex` for computing NN/IDW mappings.
- `interpolation.h`: `Interpolator` for NN/IDW interpolation.
- `regridder.h`: `Regridder` orchestrates the pipeline.
- `examples/`:
  - `example.cpp`: Example usage.
  - `CMakeLists.txt`: Builds example executable.
- `CMakeLists.txt`: Main build configuration.

## Troubleshooting

- **Error: Cannot open input file**:
  - Ensure `source.txt` and `target.txt` exist and paths are correct.
- **Error: Invalid headers**:
  - Check source/target files have matching columns (e.g., 15 for `GRID_BY_TIME`).
- **Error: No valid source points**:
  - Verify source file has valid coordinates (`|Lat| <= 90`, `|Lon| <= 360`) and values.
- **Warning: Distance exceeds radius**:
  - Increase `config.radius` or ignore if acceptable (logged when `verbose = true`).
- **Build Errors**:
  - Confirm C++17 compiler and CMake 3.10+.
  - Check all headers are in the project directory.

## License

[MIT License]

## Contributing

Contributions are welcome! Please submit issues or pull requests to the repository. For major changes, open an issue first to discuss.
