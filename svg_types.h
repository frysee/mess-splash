#ifndef SVG_TYPES_H
#define SVG_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* Point structure representing a 2D coordinate
 * Uses floating point for precise SVG path calculations
 */
typedef struct {
    float x;
    float y;
} Point;

/* Path structure representing a series of connected points
 * Can be either an outer path or a hole in another path
 */
typedef struct {
    Point *points;           // Array of points in the path
    uint32_t num_points;     // Number of points currently in use
    uint32_t capacity;       // Allocated capacity for points array
    bool is_hole;           // True if this path represents a hole
} Path;

/* Color structure representing RGBA color values */
typedef struct {
    uint8_t r;              // Red component (0-255)
    uint8_t g;              // Green component (0-255)
    uint8_t b;              // Blue component (0-255)
    uint8_t a;              // Alpha component (0-255)
} Color;

/* SVGPath structure representing a complete SVG path
 * Can contain multiple sub-paths including holes
 */
typedef struct {
    Path *paths;            // Array of paths
    uint32_t num_paths;     // Number of paths currently in use
    uint32_t capacity;      // Allocated capacity for paths array
    Color fill_color;       // Fill color for the path
} SVGPath;

#endif
