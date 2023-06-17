#include "utils/cube.hpp"


cube::cube(point3 cen, double side_len, vec3 up, vec3 front, vec3 right, std::shared_ptr<material> m){
    center = cen;
    this->side_len = side_len;
    this->up = up;
    this->front = front;
    this->right = right;
    mat_ptr = m;

    triangle triangles[12];

    // front face
    point3 fr_t_l = center + front * side_len / 2 + up * side_len / 2 - right * side_len / 2;
    point3 fr_t_r = center + front * side_len / 2 + up * side_len / 2 + right * side_len / 2;
    point3 fr_b_l = center + front * side_len / 2 - up * side_len / 2 - right * side_len / 2;
    point3 fr_b_r = center + front * side_len / 2 - up * side_len / 2 + right * side_len / 2;
    point3 bk_t_l = center - front * side_len / 2 + up * side_len / 2 - right * side_len / 2;
    point3 bk_t_r = center - front * side_len / 2 + up * side_len / 2 + right * side_len / 2;
    point3 bk_b_l = center - front * side_len / 2 - up * side_len / 2 - right * side_len / 2;
    point3 bk_b_r = center - front * side_len / 2 - up * side_len / 2 + right * side_len / 2;

    this->triangles[0] = triangle(fr_t_l, fr_t_r, fr_b_l, mat_ptr);
    this->triangles[1] = triangle(fr_t_r, fr_b_l, fr_b_r, mat_ptr);
    this->triangles[2] = triangle(bk_t_l, bk_t_r, bk_b_l, mat_ptr);
    this->triangles[3] = triangle(bk_t_r, bk_b_l, bk_b_r, mat_ptr);
    this->triangles[4] = triangle(fr_t_l, fr_t_r, bk_t_l, mat_ptr);
    this->triangles[5] = triangle(fr_t_r, bk_t_l, bk_t_r, mat_ptr);
    this->triangles[6] = triangle(fr_b_l, fr_b_r, bk_b_l, mat_ptr);
    this->triangles[7] = triangle(fr_b_r, bk_b_l, bk_b_r, mat_ptr);
    this->triangles[8] = triangle(fr_t_l, fr_b_l, bk_t_l, mat_ptr);
    this->triangles[9] = triangle(fr_b_l, bk_t_l, bk_b_l, mat_ptr);
    this->triangles[10] = triangle(fr_t_r, fr_b_r, bk_t_r, mat_ptr);
    this->triangles[11] = triangle(fr_b_r, bk_t_r, bk_b_r, mat_ptr);
}

bool cube::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    double closest_so_far = t_max;

    for (const triangle& tri : triangles) {
        if (tri.hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    rec.front_face = dot(r.direction(), rec.normal) < 0;
    rec.mat_ptr = mat_ptr;
    rec.p = r.at(rec.t);
    rec.set_face_normal(r, rec.normal);

    return hit_anything;
}
