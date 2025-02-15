#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "svg_parser.h"

#define INITIAL_CAPACITY 100
#define MAX_SUBPATHS 10

/* Track current position during path parsing */
static Point current_point = {0, 0};
static Point start_point = {0, 0};

/* Structure to handle compound paths with holes */
typedef struct {
    Path paths[MAX_SUBPATHS];
    int num_paths;
} CompoundPath;

/* Parse a floating point number from a string
 * Advances the string pointer past the parsed number
 */
static float parse_number(const char **str) {
    // Skip whitespace and commas
    while (isspace(**str) || **str == ',') (*str)++;

    char *end;
    float num = strtof(*str, &end);
    *str = end;
    return num;
}

/* Add a point to a path, growing the array if needed */
static void add_point_to_path(Path *path, float x, float y) {
    if (path->num_points >= path->capacity) {
        path->capacity *= 2;
        Point *new_points = realloc(path->points, path->capacity * sizeof(Point));
        if (!new_points) {
            return;
        }
        path->points = new_points;
    }
    path->points[path->num_points].x = x;
    path->points[path->num_points].y = y;
    path->num_points++;
}

/* Add all paths from a compound path to the SVG structure
 * First path is considered the outer path, subsequent paths are holes
 */
static void add_compound_path_to_svg(SVGPath *svg, CompoundPath *compound) {
    for (int i = 0; i < compound->num_paths; i++) {
        if (svg->num_paths >= svg->capacity) {
            svg->capacity *= 2;
            Path *new_paths = realloc(svg->paths, svg->capacity * sizeof(Path));
            if (!new_paths) {
                return;
            }
            svg->paths = new_paths;
        }
        svg->paths[svg->num_paths] = compound->paths[i];
        svg->paths[svg->num_paths].is_hole = (i > 0); // First path is outer, rest are holes
        svg->num_paths++;
    }
}

/* Parse an RGB color string into a Color structure */
Color parse_color(const char *color_str) {
    Color color = {0, 0, 0, 255}; // Default to opaque black
    
    if (strstr(color_str, "rgb(") == color_str) {
        int r, g, b;
        if (sscanf(color_str, "rgb(%d,%d,%d)", &r, &g, &b) == 3) {
            color.r = (uint8_t)r;
            color.g = (uint8_t)g;
            color.b = (uint8_t)b;
        }
    }

    return color;
}

/* Parse an SVG path data string into an SVGPath structure
 * Handles multiple subpaths and holes
 * Supports commands: M, L, H, V, C, Z
 */
SVGPath* parse_svg_path(const char *path_data, const char *style) {
    // Initialize SVG structure
    SVGPath *svg = malloc(sizeof(SVGPath));
    if (!svg) return NULL;

    svg->paths = malloc(INITIAL_CAPACITY * sizeof(Path));
    if (!svg->paths) {
        free(svg);
        return NULL;
    }

    svg->num_paths = 0;
    svg->capacity = INITIAL_CAPACITY;
    svg->fill_color = parse_color(style);

    // Initialize compound path structure
    CompoundPath compound = {0};
    compound.num_paths = 0;

    // Initialize first path
    Path *current_path = &compound.paths[0];
    current_path->points = malloc(INITIAL_CAPACITY * sizeof(Point));
    if (!current_path->points) {
        free(svg->paths);
        free(svg);
        return NULL;
    }
    current_path->num_points = 0;
    current_path->capacity = INITIAL_CAPACITY;
    current_path->is_hole = 0;

    const char *p = path_data;
    char command = 'M';
    float x1, y1, x2, y2, x3, y3;
    bool new_subpath = true;

    // Parse path commands
    while (*p) {
        if (isalpha(*p)) {
            // Handle new subpath creation
            if (*p == 'M' && !new_subpath) {
                if (current_path->num_points > 0) {
                    compound.num_paths++;
                    if (compound.num_paths < MAX_SUBPATHS) {
                        current_path = &compound.paths[compound.num_paths];
                        current_path->points = malloc(INITIAL_CAPACITY * sizeof(Point));
                        current_path->num_points = 0;
                        current_path->capacity = INITIAL_CAPACITY;
                        current_path->is_hole = 1;
                    }
                }
            }
            command = *p++;
            new_subpath = (command == 'M');
        }

        // Process commands
        switch (command) {
            case 'M': // Move To
                x1 = parse_number(&p);
                y1 = parse_number(&p);
                add_point_to_path(current_path, x1, y1);
                current_point.x = start_point.x = x1;
                current_point.y = start_point.y = y1;
                command = 'L'; // After M, implicit command is L
                break;

            case 'L': // Line To
                x1 = parse_number(&p);
                y1 = parse_number(&p);
                add_point_to_path(current_path, x1, y1);
                current_point.x = x1;
                current_point.y = y1;
                break;

            case 'H': // Horizontal Line
                x1 = parse_number(&p);
                add_point_to_path(current_path, x1, current_point.y);
                current_point.x = x1;
                break;

            case 'V': // Vertical Line
                y1 = parse_number(&p);
                add_point_to_path(current_path, current_point.x, y1);
                current_point.y = y1;
                break;

            case 'Z': // Close Path
            case 'z':
                if (current_path->num_points > 0) {
                    add_point_to_path(current_path, start_point.x, start_point.y);
                }
                break;

            case 'C': // Cubic Bezier Curve
                // Get control points and end point
                x1 = parse_number(&p);
                y1 = parse_number(&p);
                x2 = parse_number(&p);
                y2 = parse_number(&p);
                x3 = parse_number(&p);
                y3 = parse_number(&p);

                // Approximate curve with line segments
                for (float t = 0; t <= 1; t += 0.1) {
                    float t_squared = t * t;
                    float t_cubed = t_squared * t;
                    float mt = 1 - t;
                    float mt_squared = mt * mt;
                    float mt_cubed = mt_squared * mt;

                    // Cubic Bezier formula
                    float px = current_point.x * mt_cubed +
                              3 * x1 * mt_squared * t +
                              3 * x2 * mt * t_squared +
                              x3 * t_cubed;

                    float py = current_point.y * mt_cubed +
                              3 * y1 * mt_squared * t +
                              3 * y2 * mt * t_squared +
                              y3 * t_cubed;

                    add_point_to_path(current_path, px, py);
                }

                current_point.x = x3;
                current_point.y = y3;
                break;

            default:
                // Skip unknown commands
                while (*p && !isalpha(*p)) p++;
                break;
        }

        // Skip whitespace
        while (isspace(*p)) p++;
    }

    // Add final path if it contains points
    if (current_path->num_points > 0) {
        compound.num_paths++;
    }

    // Add all paths to SVG structure
    add_compound_path_to_svg(svg, &compound);

    return svg;
}

/* Free all resources associated with an SVG path */
void free_svg_path(SVGPath *svg) {
    if (svg) {
        for (uint32_t i = 0; i < svg->num_paths; i++) {
            free(svg->paths[i].points);
        }
        free(svg->paths);
        free(svg);
    }
}
