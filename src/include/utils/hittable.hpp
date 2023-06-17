#pragma once

#include "math/ray.hpp"
#include <memory>

class material;

struct hit_record {
    point3 p;
    vec3 normal;
    std::shared_ptr<material> mat_ptr;
    double t;
    bool front_face;

    inline void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
public:
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

class triangle : public hittable {
public:
    triangle() {}
    triangle(const point3 vertices[3], std::shared_ptr<material> m)
        : vertices{ vertices[0], vertices[1], vertices[2] }, mat_ptr(m) {};
    
    triangle(const point3& v0, const point3& v1, const point3& v2, std::shared_ptr<material> m)
        : vertices{ v0, v1, v2 }, mat_ptr(m) {};

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

    point3& operator[](int i) { return vertices[i]; }
    const point3& operator[](int i) const { return vertices[i]; }
    // << operator for printing
    friend std::ostream& operator<<(std::ostream& out, const triangle& tri) {
        return out << "triangle: " << tri[0] << ", " << tri[1] << ", " << tri[2];
    }

public:
    vec3 vertices[3]; // [0] is the top vertex, [1] is the bottom left vertex, [2] is the bottom right vertex
    std::shared_ptr<material> mat_ptr;
};

bool ray_triangle_intersection(const ray& r, const triangle tri, double t_min, double t_max, hit_record& rec);