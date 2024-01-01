#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <omp.h>
#include <math.h>
#include <limits.h>

#include "cubiomes/generator.h"
#include "cubiomes/rng.h"

double expected_deviation(int p_75385_) {
    return 0.1D * (1.0D + 1.0D / (double)(p_75385_ + 1));
}

uint64_t lfloor(double p_14135_) {
    uint64_t i = (uint64_t)p_14135_;
    return p_14135_ < (double)i ? i - 1L : i;
}

double wrap(double p_75407_) {
    return p_75407_ - (double)lfloor(p_75407_ / 3.3554432E7D + 0.5D) * 3.3554432E7D;
}

typedef struct {
    double *values;
    size_t size;
} Amplitudes;

Amplitudes amplitudes_new(double *values, size_t size) {
    return (Amplitudes){.values=values, .size=size};
}

typedef struct {
    int first_octave;
    Amplitudes amplitudes;
    PerlinNoise *noise_levels;
    double lowest_freq_input_factor;
    double lowest_freq_value_factor;
    double max_value;
} PerlinNoiseLayer;

double edge_value(PerlinNoiseLayer layer, double p_210650_) {
    double d0 = 0.0D;
    double d1 = layer.lowest_freq_value_factor;

    for (int i = 0; i < 3; ++i) {
        d0 += layer.amplitudes.values[i] * p_210650_ * d1;
        d1 /= 2.0D;
    }

    return d0;
}

PerlinNoiseLayer perlin_noise_layer_new(Xoroshiro *xr, int first_octave, Amplitudes amplitudes) {
    PerlinNoiseLayer final;
    final.first_octave = first_octave;
    final.amplitudes = amplitudes;

    int i = 3;
    int j = -final.first_octave;
    PerlinNoise *noise_levels = (PerlinNoise *)malloc(sizeof(PerlinNoise) * amplitudes.size);

    Xoroshiro random = xForkPositional(xr);

    for (int k = 0; k < amplitudes.size; k++) {
        if (final.amplitudes.values[k] != 0.0D) {
            int l = final.first_octave + k;

            char *string;
            if (l == -6) {
                string = "octave_-6";
            }
            else if (l == -5) {
                string = "octave_-5";
            }
            else if (l == -4) {
                string = "octave_-4";
            }

            Xoroshiro positional = xFromHashOf(&random, string);
            xPerlinInit(&noise_levels[k], &positional);
        }
    }

    final.noise_levels = noise_levels;

    final.lowest_freq_input_factor = pow(2.0D, (double)(-j));
    final.lowest_freq_value_factor = pow(2.0D, (double)(i - 1)) / (pow(2.0D, (double)i) - 1.0D);
    final.max_value = edge_value(final, 2.0D);

    return final;
}

double perlin_noise_layer_get_value_internal(PerlinNoiseLayer noise, double p_75418_, double p_75419_, double p_75420_, double p_75421_, double p_75422_, bool p_75423_) {
    double d0 = 0.0D;
    double d1 = noise.lowest_freq_input_factor;
    double d2 = noise.lowest_freq_value_factor;

    for (int i = 0; i < noise.amplitudes.size; i++) {
        PerlinNoise pnoise = noise.noise_levels[i];
        
        double d3 = samplePerlin(&pnoise, wrap(p_75418_ * d1), wrap(p_75419_ * d1), wrap(p_75420_ * d1), p_75421_ * d1, p_75422_ * d1);
        d0 += noise.amplitudes.values[i] * d3 * d2;

        d1 *= 2.0D;
        d2 /= 2.0D;
    }
    return d0;
}

double perlin_noise_layer_get_value(PerlinNoiseLayer noise, double d0, double d1, double d2) {
    return perlin_noise_layer_get_value_internal(noise, d0, d1, d2, 0.0D, 0.0D, false);
}

typedef struct {
    int first_octave;
    Amplitudes amplitudes;
    PerlinNoiseLayer first;
    PerlinNoiseLayer second;
    Xoroshiro xr;
    double value_factor;
    double max_value;
} NormalNoise;

NormalNoise normal_noise_new(Xoroshiro *xr, int first_octave, Amplitudes amplitudes, char *name) {
    Xoroshiro normal_xr = xFromHashOf(xr, name);

    NormalNoise final;
    final.first_octave = first_octave; 
    final.amplitudes = amplitudes;
    final.xr = normal_xr;
    
    final.first = perlin_noise_layer_new(&final.xr, final.first_octave, final.amplitudes);
    final.second = perlin_noise_layer_new(&final.xr, final.first_octave, final.amplitudes);

    int j = INT_MAX;
    int k = INT_MIN;

    for (int l = 0; l < amplitudes.size; l++) {
        double d0 = amplitudes.values[l];
        if (d0 != 0.0D) {
            j = fmin(j, l);
            k = fmax(k, l);
        }
    }

    final.value_factor = 0.16666666666666666D / expected_deviation(k - j);
    final.max_value = (final.first.max_value + final.second.max_value) * final.value_factor;

    return final;
}

double get_value(NormalNoise *noise, double p_75381_, double p_75382_, double p_75383_) {
    double d0 = p_75381_ * 1.0181268882175227D;
    double d1 = p_75382_ * 1.0181268882175227D;
    double d2 = p_75383_ * 1.0181268882175227D;
    
    PerlinNoiseLayer n0 = noise->first;
    PerlinNoiseLayer n1 = noise->second;

    return (perlin_noise_layer_get_value(n0, p_75381_, p_75382_, p_75383_) + perlin_noise_layer_get_value(n1, d0, d1, d2)) * noise->value_factor;
}

int get_dirt_height(Xoroshiro *random, NormalNoise *noise, double x, double z) {
    double d0 = get_value(noise, x, 0.0D, z);
    Xoroshiro xr = xAt(random, x, 0, z);
    double rand = xNextDouble(&xr);
    double d = (d0 * 2.75D + 3.0D + rand * 0.25D);
    return (int)(d+1);
}   

int height_at(int seed, int x, int y) {
    Xoroshiro xr;
    xSetSeed(&xr, seed);

    Xoroshiro random = xForkPositional(&xr);

    double primary_amplitudes[3] = {1.0, 1.0, 1.0};
    NormalNoise noise = normal_noise_new(&random, -6, amplitudes_new(primary_amplitudes, 3), "minecraft:surface");
    
    int height = get_dirt_height(&random, &noise, (double)x, (double)y);

    free(noise.first.noise_levels);
    free(noise.second.noise_levels);

    return height;
}