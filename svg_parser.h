#ifndef SVG_PARSER_H
#define SVG_PARSER_H

#include "svg_types.h"

/* Parse an SVG path string into an SVGPath structure
 * path_data: SVG path data string (e.g., "M 0,0 L 100,100 Z")
 * style: CSS style string containing color information
 * Returns: Pointer to parsed SVGPath structure or NULL on failure
 */
SVGPath* parse_svg_path(const char *path_data, const char *style);

/* Free resources associated with an SVGPath structure */
void free_svg_path(SVGPath *path);

/* Parse a color string into a Color structure
 * Supports RGB format (e.g., "rgb(255,0,0)")
 */
Color parse_color(const char *color_str);

#endif
