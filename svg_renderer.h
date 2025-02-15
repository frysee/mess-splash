#ifndef SVG_RENDERER_H
#define SVG_RENDERER_H

#include "fbsplash.h"
#include "svg_types.h"

/* Render an SVG path to the framebuffer
 * Handles multiple paths and holes, applies scaling and centering
 */
void render_svg_path(Framebuffer *fb, SVGPath *svg, DisplayInfo *display_info);

/* Rotate an SVG path by the specified angle
 * angle: Must be 90, 180, or 270 degrees
 */
void rotate_svg_path(SVGPath *svg, int angle);

#endif
