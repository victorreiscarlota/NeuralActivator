#include <stdio.h>

#define WIDTH 20
#define HEIGHT 20

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
    int y0 = clmapi(y, 0, HEIGHT-1);
    int x1 = clampi(x0 + w - 1, 0, WIDTH-1);
    int y1 = clampi(y0 + h - 1, 0, HEIGHT-1);
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
    float output = feedForward(inputs, weights);
    printf("Output: %f\n", output);
    return 0;
}
