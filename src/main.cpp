#include <thread>
#include <vector>
#include <atomic>
#include <iomanip>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "pix/pix.hpp"
#include "math/utils.hpp"
#include "utils/sphere.hpp"
#include "utils/cube.hpp"
#include "utils/hittable_list.hpp"
#include "utils/camera.hpp"
#include "utils/material.hpp"

const int num_threads = std::thread::hardware_concurrency();
std::vector<std::thread> threads(num_threads);
std::atomic<int> scanlines_processed = 0;

const auto aspect_ratio = 16.0 / 9.0;
const int image_width = 800;
const int image_height = static_cast<int>(image_width / aspect_ratio);
point3 lookfrom(3, 2, 10);
point3 lookat(0, 0, -1);
vec3 vup(0, 1, 0);
auto dist_to_focus = (lookat - lookfrom).length();
auto aperture = 0.1;

camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

const int samples_per_pixel = 20;
const int max_depth = 50;

hittable_list world;

double lastTime = 0.0;
int frame_count = 0;

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
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
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

    //rotate camera around the lookat point
    lookfrom = point3(10 * cos(frame_count * 0.1), 2, 10 * sin(frame_count * 0.1));
    cam = camera(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    for (int t = 0; t < num_threads; t++)
    {
        threads[t] = std::thread([&](int thread_id)
                                 {
            for (int j = pix->height - 1 - thread_id; j >= 0; j -= num_threads)
            {
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
                scanlines_processed++;
            } },
                                 t);
    }

    while (scanlines_processed < pix->height)
    {
        int remaining = pix->height - scanlines_processed;
        // Percentage of scanlines processed upto 2 decimal places
        std::cout << "Scanlines remaining: " << remaining << " : Remaining " << std::fixed << std::setprecision(2) << (remaining * 100.0) / pix->height << "%"
                  << "\r";
    }

    for (int t = 0; t < num_threads; t++)
    {
        threads[t].join();
    }

    std::cout << "Frame : " << frame_count << " FPS : " << 1.0 / delta << " Frame time : " << delta << std::endl;

    frame_count++;
    scanlines_processed = 0;
    if (!save_image)
        return;
    // Check if output folder exists
    std::string file_name = "output/frame_" + std::to_string(frame_count) + ".jpg";
    // Flip image
    stbi_flip_vertically_on_write(true);
    stbi_write_jpg(file_name.c_str(), pix->width, pix->height, 3, pix->pixels, 100);
}

hittable_list random_scene()
{
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9)
            {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8)
                {
                    // diffuse
                    auto albedo = random_vec3() * random_vec3();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    auto albedo = random_vec3(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<cube>(point3(0, 1, 0), 2, vec3(0, 1, 0), vec3(1, 0, 0), material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<cube>(point3(-4, 1, 0), 3, vec3(0, 1, 0), vec3(1, 0, 0), material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.1);
    world.add(make_shared<cube>(point3(4, 1, 0), 1, vec3(0, 1, 1), vec3(1, 0, 0), material3));

    return world;
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

    world = random_scene();

    auto yellow = "\u001b[33m";
    auto reset = "\u001b[0m";
    std::cout << yellow << "Using " << num_threads << " threads" << reset << std::endl;

    pix.PixRun(renderCallback);
    return 0;
}
