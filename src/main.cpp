#include "pix/pix.hpp"

// Global variables
int stripeOffset = 0;        // Tracks the offset of the stripes
double lastUpdateTime = 0;   // Tracks the time of the last update
double movementSpeed = 20.0; // Number of pixels to move per second

void renderCallback(Pix *pix)
{
    double currentTime = Pix::GetTime();
    double deltaTime = currentTime - lastUpdateTime;

    if (deltaTime < 1.0)
    {
        return;
    }
    lastUpdateTime = currentTime;

    // Moving red, green, blue stripes 20 pixels wide, moving from right to left
    for (int y = 0; y < pix->height; y++)
    {
        for (int x = 0; x < pix->width; x += 20)
        {
            int stripeIndex = (x + stripeOffset) / 20;

            int r = 0;
            int g = 0;
            int b = 0;

            switch (stripeIndex % 3)
            {
            case 0:
                r = 255; // Red stripe
                break;
            case 1:
                g = 255; // Green stripe
                break;
            case 2:
                b = 255; // Blue stripe
                break;
            }

            // Set the color values for each pixel in the stripe
            for (int i = x; i < x + 20; i++)
            {
                pix->SetPixel(i, y, r, g, b);
            }
        }
    }

    // Update stripe offset for the next frame
    int pixelsToMove = (int)(movementSpeed * deltaTime);
    stripeOffset = (stripeOffset + pixelsToMove) % pix->width;
}

int main()
{
    auto pix = Pix(800, 600, "Pixren");
    pix.PixRun(renderCallback);
    return 0;
}