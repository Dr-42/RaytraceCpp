#pragma once

#include <math/utils.hpp>

class camera {
public:
    camera(
        point3 lookfrom,
        point3 lookat,
        vec3 vup,
        double vfov, // vfov is top to bottom in degrees
        double aspect_ratio,
        double aperture,
        double focus_dist
    );
    ray get_ray(double u, double v) const;

private:
    point3 origin;
    point3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    double lens_radius;
};