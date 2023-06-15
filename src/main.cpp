#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "pix/pix.hpp"
#include "math/utils.hpp"
#include "utils/sphere.hpp"
#include "utils/hittable_list.hpp"
#include "utils/camera.hpp"

const auto aspect_ratio = 16.0 / 9.0;
const int image_width = 1080;
const int image_height = static_cast<int>(image_width / aspect_ratio);
const int samples_per_pixel = 1;

hittable_list world;
camera cam;

auto viewport_height = 2.0;
auto viewport_width = aspect_ratio * viewport_height;
auto focal_length = 1.0;

auto origin = point3(0, 0, 0);
auto horizontal = vec3(viewport_width, 0, 0);
auto vertical = vec3(0, viewport_height, 0);
auto lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);

double lastTime = 0.0;
int frame_count = 1;

bool save_image = false;

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
        0, //sin(degrees_to_radians(frame_count) * 2) * 0.5,
        -1);

    for (int j = pix->height - 1; j >= 0; --j)
    {
        for (int i = 0; i < pix->width; ++i)
        {
            color pixel_color(0, 0, 0);
            //Anti-aliasing
            //Replace frame_count with samples_per_pixel for steady image
            for (int s = 0; s < frame_count; s++){
                auto u = (i + random_double()) / (pix->width - 1);
                auto v = (j + random_double()) / (pix->height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world);
            }
            pix->SetPixel(i, j, pixel_color, frame_count);
        }
    }
    frame_count++;
    if (!save_image)
        return;
    //Check if output folder exists
    std::string file_name = "output/frame_" + std::to_string(frame_count) + ".jpg";
    //Flip image
    stbi_flip_vertically_on_write(true);
    stbi_write_jpg(file_name.c_str(), pix->width, pix->height, 3, pix->pixels, 100);
}

int main(int argc, char const *argv[])
{
    if(argc > 1){
        if (strcmp(argv[1], "--save") == 0)
        {
            save_image = true;
            std::cout << "Saving images to output folder. Make sure a folder named \"output\" exists..." << std::endl;
        }
    }

    auto pix = Pix(image_width, image_height, "Raytracer");

    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));

    pix.PixRun(renderCallback);
    return 0;
}
