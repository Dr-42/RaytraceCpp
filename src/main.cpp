#include "pix/pix.hpp"
#include "math/utils.hpp"
#include "utils/sphere.hpp"
#include "utils/hittable_list.hpp"

const auto aspect_ratio = 16.0 / 9.0;
const int image_width = 1080;
const int image_height = static_cast<int>(image_width / aspect_ratio);

hittable_list world;

auto viewport_height = 2.0;
auto viewport_width = aspect_ratio * viewport_height;
auto focal_length = 1.0;

auto origin = point3(0, 0, 0);
auto horizontal = vec3(viewport_width, 0, 0);
auto vertical = vec3(0, viewport_height, 0);
auto lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);

double lastTime = 0.0;

color ray_color(const ray &r, const hittable &world)
{
    hit_record rec;
    if (world.hit(r, 0, infinity, rec))
    {
        return 0.5 * (rec.normal + color(1, 1, 1));
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1, 1, 1) + t * color(0.5, 0.7, 1);
}

void renderCallback(Pix *pix)
{
    //Print framerate
    double currentTime = Pix::GetTime();
    double delta = currentTime - lastTime;
    lastTime = currentTime;
    std::cout << "FPS: " << 1.0 / delta << std::endl;

    auto sph = dynamic_cast<sphere *>(world.objects[world.objects.size() - 1].get());
    sph->center = point3(
        0,
        sin(Pix::GetTime() * 2) * 0.5,
        -1);

    for (int j = pix->height - 1; j >= 0; --j)
    {
        for (int i = 0; i < pix->width; ++i)
        {
            auto u = double(i) / (pix->width - 1);
            auto v = double(j) / (pix->height - 1);
            ray r(origin, lower_left_corner + u * horizontal + v * vertical);
            color col = ray_color(r, world);

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

    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));

    pix.PixRun(renderCallback);
    return 0;
}
