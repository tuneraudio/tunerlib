#ifndef TUNER_BIQUAD_H
#define TUNER_BIQUAD_H

typedef double smp_t;

/* filter types */
typedef enum {
    FILTER_LOW_PASS,
    FILTER_HIGH_PASS,
    FILTER_BAND_PASS,
    FILTER_NOTCH,
    FILTER_PEAKING_BAND,
    FILTER_LOW_SHELF,
    FILTER_HIGH_SHELF,
    LAST_FILTER
} filter_type_t;

/* filter representation */
typedef struct filter {
    filter_type_t type;
    smp_t gain;         /* gain in dB */
    smp_t fc;           /* cut off / center frequency */
    smp_t fs;           /* sample rate */
    smp_t bw;           /* bandwidth in octaves */
} filter_t;

typedef struct biquad biquad_t;

biquad_t *biquad_new(filter_t *f);
int biquad_init(biquad_t *b, filter_t *f);

smp_t df1(smp_t sample, biquad_t *b);

#endif
