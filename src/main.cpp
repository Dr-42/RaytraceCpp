#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "pix/pix.hpp"
#include "math/utils.hpp"
#include "utils/sphere.hpp"
#include "utils/hittable_list.hpp"
#include "utils/camera.hpp"
#include "utils/material.hpp"

const auto aspect_ratio = 16.0 / 9.0;
const int image_width = 1080;
const int image_height = static_cast<int>(image_width / aspect_ratio);

const int samples_per_pixel = 100;
const int max_depth = 50;

hittable_list world;
camera cam(point3(-1,1,0), point3(0,0,-1), vec3(0,1,0), 100, aspect_ratio);

double lastTime = 0.0;
int frame_count = 1;

bool save_image = false;

color ray_color(const ray &r, const hittable &world, int depth)
{
    hit_record rec;

    if (depth <= 0)
    {
        return color(0, 0, 0);
    }

    if (world.hit(r, 0.001, infinity, rec))
    {
        ray scattered;
        color attenuation;
        if(rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1, 1, 1) + t * color(0.5, 0.7, 1);
}

void renderCallback(Pix *pix)
{
    // Print framerate
    double currentTime = Pix::GetTime();
    double delta = currentTime - lastTime;
    lastTime = currentTime;
    std::cout << "Frame : "<< frame_count<< " FPS : " << 1.0 / delta << " Frame time : " << delta << std::endl;

    for (int j = pix->height - 1; j >= 0; --j)
    {
        std::cout << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < pix->width; ++i)
        {
            color pixel_color(0, 0, 0);
            // Anti-aliasing
            // Replace frame_count with samples_per_pixel for steady image
            for (int s = 0; s < samples_per_pixel; s++)
            {
                auto u = (i + random_double()) / (pix->width - 1);
                auto v = (j + random_double()) / (pix->height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }
            pix->SetPixel(i, j, pixel_color, samples_per_pixel);
        }
    }
    frame_count++;
    if (!save_image)
        return;
    // Check if output folder exists
    std::string file_name = "output/frame_" + std::to_string(frame_count) + ".jpg";
    // Flip image
    stbi_flip_vertically_on_write(true);
    stbi_write_jpg(file_name.c_str(), pix->width, pix->height, 3, pix->pixels, 100);
}

int main(int argc, char const *argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "--save") == 0)
        {
            save_image = true;
            std::cout << "Saving images to output folder. Make sure a folder named \"output\" exists..." << std::endl;
        }
    }

    auto pix = Pix(image_width, image_height, "Raytracer");

    auto material_ground = make_shared<metal>(color(0.6, 0.0, 0.8), 0.2);
    auto material_center = make_shared<lambertian>(color(0.2, 0.8, 0.3));
    //auto material_left   = make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);
    auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 0.3);
    auto material_left   = make_shared<dielectric>(1.5);

    world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.0),   0.5, material_center));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
    world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));

    pix.PixRun(renderCallback);
    return 0;
}
