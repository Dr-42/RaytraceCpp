#include "pix/pix.hpp"
#include "math_utils/vec3.hpp"
#include "math_utils/ray.hpp"

const auto aspect_ratio = 16.0 / 9.0;
const int image_width = 800;
const int image_height = static_cast<int>(image_width / aspect_ratio);

auto viewport_height = 2.0;
auto viewport_width = aspect_ratio * viewport_height;
auto focal_length = 1.0;

auto origin = point3(0, 0, 0);
auto horizontal = vec3(viewport_width, 0, 0);
auto vertical = vec3(0, viewport_height, 0);
auto lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);

color ray_color(const ray &r)
{
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

void renderCallback(Pix *pix)
{
    for (int j = pix->height - 1; j >= 0; --j)
    {
        for (int i = 0; i < pix->width; ++i)
        {
            auto u = double(i) / (pix->width - 1);
            auto v = double(j) / (pix->height - 1);
            ray r(origin, lower_left_corner + u * horizontal + v * vertical - origin);
            color col = ray_color(r);

            int ir = static_cast<int>(255.999 * col.x());
            int ig = static_cast<int>(255.999 * col.y());
            int ib = static_cast<int>(255.999 * col.z());

            pix->SetPixel(i, j, ir, ig, ib);
        }
    }
}

int main()
{
    auto pix = Pix(image_width, image_height, "Raytracer");
    pix.PixRun(renderCallback);
    return 0;
}