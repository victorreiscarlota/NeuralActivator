#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <direct.h>


#include <sys/stat.h>
#include <sys/types.h>

#include "./config.h"

typedef float Layer[HEIGHT][WIDTH];

static inline int clampi(int x, int low, int high)
{
    if (x < low)
        x = low;
    if (x > high)
        x = high;
    return x;
}

void layerDrawRect(Layer layer, int x, int y, int w, int h, float value)
{
    assert(w > 0);
    assert(h > 0);
    int x0 = clampi(x, 0, WIDTH - 1);
    int y0 = clampi(y, 0, HEIGHT - 1);
    int x1 = clampi(x0 + w - 1, 0, WIDTH - 1);
    int y1 = clampi(y0 + h - 1, 0, HEIGHT - 1);

    for (int xi = x0; xi <= x1; xi++)
    {
        for (int yi = y0; yi <= y1; yi++)
        {
            layer[yi][xi] = value;
        }
    }
}

void layerDrawCircle(Layer layer, int cx, int cy, int r, float value)
{
    assert(r > 0);
    int x0 = clampi(cx - r, 0, WIDTH - 1);
    int y0 = clampi(cy - r, 0, HEIGHT - 1);
    int x1 = clampi(cx + r, 0, WIDTH - 1);
    int y1 = clampi(cy + r, 0, HEIGHT - 1);
    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            int dx = x - cx;
            int dy = y - cy;
            if (dx * dx + dy * dy <= r * r)
            {
                layer[y][x] = value;
            }
        }
    }
}

void layerSaveAsPPM(Layer layer, const char *file_path)
{

    FILE *f = fopen(file_path, "wb");

    if (f == NULL)
    {
        fprintf(stderr, "Error opening file %s: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fprintf(f, "P6\n%d %d 255\n", WIDTH * PPM_SCALER, HEIGHT * PPM_SCALER);

    for (int y = 0; y < HEIGHT * PPM_SCALER; ++y) {
        for (int x = 0; x < WIDTH * PPM_SCALER; ++x) {
            float s = (layer[y / PPM_SCALER][x / PPM_SCALER] + PPM_RANGE) / (2.0f * PPM_RANGE);
            char pixel[3] = {
                (char) floorf(PPM_COLOR_INTENSITY * (1.0f - s)),
                (char) floorf(PPM_COLOR_INTENSITY * (1.0f - s)),
                (char) floorf(PPM_COLOR_INTENSITY * s),
            };

            fwrite(pixel, sizeof(pixel), 1, f);
        }
    }

    fclose(f);
}

void layerSaveAsBin(Layer layer, const char *file_path)
{
    FILE *f = fopen(file_path, "wb");
    if (f == NULL)
    {
        fprintf(stderr, "Error opening file %s: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fwrite(layer, sizeof(Layer), 1, f);
    fclose(f);
}

void layerLoadFromBin(Layer layer, const char *file_path)
{
    (void) layer;
    (void) file_path;
    assert(0 && "TODO: layerLoadFromBin is not implemented yet!");
}

float feedForward(Layer inputs, Layer weights)
{
    float outputs = 0.0f;

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            outputs += inputs[y][x] * weights[y][x];
        }
    }

    return outputs;
}

void addInputsToWeights(Layer inputs, Layer weights)
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            weights[y][x] += inputs[y][x];
        }
    }
}

void subInputsFromWeights(Layer inputs, Layer weights)
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            weights[y][x] -= inputs[y][x];
        }
    }
}

int randRange(int low, int high)
{
    assert(low < high);
    return rand() % (high - low) + low;
}

void layerRandomRect(Layer layer)
{
    layerDrawRect(layer, 0, 0, WIDTH, HEIGHT, 0.0f);
    int x = randRange(0, WIDTH);
    int y = randRange(0, HEIGHT);

    int w = WIDTH - x;
    if (w < 2)
        w = 2;
    w = randRange(1, w);

    int h = HEIGHT - y;
    if (h < 2)
        h = 2;
    h = randRange(1, h);
    layerDrawRect(layer, x, y, w, h, 1.0f);
}

void layerRandomCircle(Layer layer)
{
    layerDrawRect(layer, 0, 0, WIDTH, HEIGHT, 0.0f);
    int cx = randRange(0, WIDTH);
    int cy = randRange(0, HEIGHT);
    int r = INT_MAX;
    if (r > cx)
        r = cx;
    if (r > cy)
        r = cy;
    if (r > WIDTH - cx)
        r = WIDTH - cx;
    if (r > HEIGHT - cy)
        r = HEIGHT - cy;
    if (r < 2)
        r = 2;
    r = randRange(1, r);
    layerDrawCircle(layer, cx, cy, r, 1.0f);
}




static Layer inputs;
static Layer weights;

float calculateAverageOutput(Layer inputs, Layer weights)
{
    float sum = 0.0f;
    
    for (int i = 0; i < SAMPLE_SIZE; i++)
    {
        layerRandomRect(inputs);
        sum += feedForward(inputs, weights);

        layerRandomCircle(inputs);
        sum += feedForward(inputs, weights);
    }

    return sum / (SAMPLE_SIZE * 2.0);
}

float calculateStandardDeviation(Layer inputs, Layer weights, float average)
{
    float sum = 0.0f;
    float output = 0.0f;

    for (int i = 0; i < SAMPLE_SIZE; i++)
    {
        layerRandomRect(inputs);
        output = feedForward(inputs, weights);
        sum += (output - average) * (output - average);

        layerRandomCircle(inputs);
        output = feedForward(inputs, weights);
        sum += (output - average) * (output - average);
    }

    return sqrt(sum / (SAMPLE_SIZE * 2.0));
}


int trainPass(Layer inputs, Layer weights)
{
    static char file_path[256];
    static int count = 0;

    int adjusted = 0;

    for (int i = 0; i < SAMPLE_SIZE; ++i) {
        layerRandomRect(inputs);
        if (feedForward(inputs, weights) > BIAS) {
            subInputsFromWeights(inputs, weights);
            snprintf(file_path, sizeof(file_path), DATA_FOLDER"/weights-%03d.ppm", count++);
            printf("[INFO] saving %s\n", file_path);
            layerSaveAsPPM(weights, file_path);
            adjusted += 1;
        }

        layerRandomCircle(inputs);
        if (feedForward(inputs, weights) < BIAS) {
            addInputsToWeights(inputs, weights);
            snprintf(file_path, sizeof(file_path), DATA_FOLDER"/weights-%03d.ppm", count++);
            printf("[INFO] saving %s\n", file_path);
            layerSaveAsPPM(weights, file_path);
            adjusted += 1;
        }
    }

    return adjusted;
}

int checkPass(Layer inputs, Layer weights)
{
    int adjusted = 0;
    for (int i = 0; i < SAMPLE_SIZE; i++)
    {
        layerRandomRect(inputs);
        if (feedForward(inputs, weights) > BIAS)
        {

            adjusted += 1;
        }
        layerRandomCircle(inputs);
        if (feedForward(inputs, weights) < BIAS)
        {

            adjusted += 1;
        }
    }
    return adjusted;
}

int main(void)
{
    printf("[INFO] creating %s\n", DATA_FOLDER);
    if (mkdir(DATA_FOLDER) < 0 && errno != EEXIST) {
        fprintf(stderr, "ERROR: could not create folder %s: %s", DATA_FOLDER,
                strerror(errno));
        exit(1);
    }

    srand(CHECK_SEED);
    int adj = checkPass(inputs, weights);
    printf("[INFO] fail rate of untrained model is %f\n", adj / (SAMPLE_SIZE * 2.0));

    for (int i = 0; i < TRAIN_PASSES; ++i) {
        srand(TRAIN_SEED);
        int adj = trainPass(inputs, weights);
        printf("[INFO] Pass %d: adjusted %d times\n", i, adj);
        if (adj <= 0) break;
    }

    srand(CHECK_SEED);
    adj = checkPass(inputs, weights);
    printf("[INFO] fail rate of trained model is %f\n", adj / (SAMPLE_SIZE * 2.0));

    return 0;
}
