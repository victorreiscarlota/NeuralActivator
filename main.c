#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <math.h>  

#define PPM_SCALER 10
#define WIDTH 23
#define HEIGHT 23


typedef float Layer[HEIGHT][WIDTH];

static inline int clampi (int x, int low, int high) 
{
    if (x < low) x = low;
    if (x > high) x = high;
    return x;
    
    
}

void layerDrawRect(Layer layer, int x, int y, int w, int h, float value) 
{
    assert (w > 0);
    assert (h > 0);
    int x0 = clampi(x, 0, WIDTH-1);
    int y0 = clampi(y, 0, HEIGHT-1);
    int x1 = clampi(x0 + w - 1, 0, WIDTH-1);
    int y1 = clampi(y0 + h - 1, 0, HEIGHT-1);

    for (int xi = x0; xi <= x1; xi++){
        for (int yi=y0; yi <=y1; yi++){
            layer[yi][xi] = value;
        }
    }
}

void layerDrawCircle(Layer layer, int cx, int cy, int r, float value)
{
    assert (r > 0);
    int x0 = clampi(cx - r, 0, WIDTH-1);
    int y0 = clampi(cy - r, 0, HEIGHT-1);
    int x1 = clampi(cx + r, 0, WIDTH-1);
    int y1 = clampi(cy + r, 0, HEIGHT-1);

    for (int yi = y0; yi <= y1; yi++){
        for (int xi = x0; xi <= x1; xi++){
            float dx = xi - cx;
            float dy = yi - cy;
            float d = sqrtf(dx*dx + dy*dy);
            if (d <= r){
                layer[yi][xi] = value;
            }
        }
    }
}

void layerSavePPM(Layer layer, const char *file_path)
{
    FILE * f = fopen(file_path, "wb");

    if (f == NULL) {
        fprintf(stderr, "Error opening file %s: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fprintf(f, "P6\n%d %d 255\n", WIDTH * PPM_SCALER, HEIGHT * PPM_SCALER);
    
    for (int y = 0; y < HEIGHT * PPM_SCALER; y++) {
        for (int x = 0; x < WIDTH * PPM_SCALER; x++){
            float s = layer[y / PPM_SCALER][x / PPM_SCALER];
            char pixel [3] = {
                (char) floorf(255 * s),
                0,
                0
            };
                fwrite(pixel, sizeof(pixel), 1, f);
        }
    }

    fclose(f);
}

float feedForward(Layer inputs, Layer weights)
{
    float outputs = 0.0f;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            outputs += inputs[y][x] * weights[y][x];
        }
    }
    
    return outputs;
}

static Layer inputs;
static Layer weights;



int main(void)
{
    
    layerDrawCircle(inputs, WIDTH/2, HEIGHT/2, WIDTH/2, 1.0f);
    layerSavePPM(inputs, "inputs.ppm");
    // float output = feedForward(inputs, weights);
    // printf("Output: %f\n", output);
    
    return 0;
}
