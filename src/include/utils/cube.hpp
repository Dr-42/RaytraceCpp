#pragma once

#include "utils/hittable.hpp"
#include "math/vec3.hpp"
#include <memory>
#include <vector>

class cube : public hittable
{
public:
    cube(point3 cen, double side_len, vec3 up, vec3 front, vec3 right, std::shared_ptr<material> m);

    virtual bool hit(
        const ray &r, double t_min, double t_max, hit_record &rec) const override;

public:
    point3 center;
    double side_len;
    vec3 up, front, right;
    triangle triangles[12];
    std::shared_ptr<material> mat_ptr;
};