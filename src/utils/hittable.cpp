#include "utils/hittable.hpp"

bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    return ray_triangle_intersection(r, *this, t_min, t_max, rec);
}

bool ray_triangle_intersection(const ray& r, const triangle tri, double t_min, double t_max, hit_record& rec) {
    // Möller-Trumbore algorithm
    vec3 edge1 = tri[1] - tri[0];
    vec3 edge2 = tri[2] - tri[0];
    vec3 h = cross(r.direction(), edge2);
    double a = dot(edge1, h);

    if (a > -t_min && a < t_min)
        return false;

    double f = 1.0 / a;
    vec3 s = r.origin() - tri[0];
    double u = f * dot(s, h);

    if (u < 0.0 || u > 1.0)
        return false;

    vec3 q = cross(s, edge1);
    double v = f * dot(r.direction(), q);

    if (v < 0.0 || u + v > 1.0)
        return false;

    double t = f * dot(edge2, q);

    if (t > t_min && t < t_max) {
        rec.t = t;
        rec.p = r.at(rec.t);
        rec.set_face_normal(r, cross(edge1, edge2));
        return true;
    }

    return false;
}