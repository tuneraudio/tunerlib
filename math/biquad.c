#include "math/biquad.h"

#include <stdlib.h>
#include <errno.h>
#include <math.h>

struct biquad {
    smp_t b0, b1, b2, a1, a2;
    smp_t x1, x2, y1, y2;
};

typedef struct {
    smp_t a0, a1, a2, b0, b1, b2;
} f_t;

typedef void (* filter_cb)(smp_t cs, smp_t alpha, smp_t A, f_t *data);

static void low_pass(smp_t cs, smp_t alpha, smp_t A, f_t *data);
static void high_pass(smp_t cs, smp_t alpha, smp_t A, f_t *data);
/* void band_pass(smp_t cs, smp_t alpha, smp_t A, f_t *data); */
/* void notch(smp_t cs, smp_t alpha, smp_t A, f_t *data); */
/* void peaking_band(smp_t cs, smp_t alpha, smp_t A, f_t *data); */
/* void low_shelf(smp_t cs, smp_t alpha, smp_t A, f_t *data); */
/* void high_shelf(smp_t cs, smp_t alpha, smp_t A, f_t *data); */

smp_t
df1(smp_t sample, biquad_t *b)
{
    smp_t ret = b->b0 * sample;

    ret += b->b1 * b->x1;
    ret += b->b2 * b->x2;
    ret -= b->a1 * b->y1;
    ret -= b->a2 * b->y2;

    b->x2 = b->x1;
    b->x1 = sample;

    b->y2 = b->y1;
    b->y1 = ret;

    return ret;
}

void
low_pass(smp_t cs, smp_t alpha, smp_t A, f_t *data)
{
    data->b0 = (1 - cs) / 2;
    data->b1 =  1 - cs;
    data->b2 = (1 - cs) / 2;
    data->a0 =  1 + alpha;
    data->a1 = -2 * cs;
    data->a2 =  1 - alpha;
}

void
high_pass(smp_t cs, smp_t alpha, smp_t A, f_t *data)
{
    data->b0 =  (1 + cs) / 2;
    data->b1 = -(1 + cs);
    data->b2 =  (1 + cs) / 2;
    data->a0 =   1 + alpha;
    data->a1 =  -2 * cs;
    data->a2 =   1 - alpha;
}

static filter_cb filters[LAST_FILTER] = {
    [FILTER_LOW_PASS ] = low_pass,
    [FILTER_HIGH_PASS] = high_pass,
    NULL
};

int
biquad_compute(biquad_t *b, filter_t *filter)
{
    if (!filters[filter->type])
        return -EINVAL;

    smp_t A, omega, sn, cs, alpha, beta;
    f_t data;

    A = pow(10, filter->gain / 40);
    omega = 2 * M_PI * filter->fc / filter->fs;
    sn = sin(omega);
    cs = cos(omega);
    alpha = sn * sinh(M_LN2 / 2 * filter->bw * omega / sn);
    beta = sqrt(A + A);

    /* apply the filter */
    filters[filter->type](cs, alpha, A, &data);

    /* precompute the coefficients */
    b->b0 = data.b0 / data.a0;
    b->b1 = data.b1 / data.a0;
    b->b2 = data.b2 / data.a0;
    b->a1 = data.a1 / data.a0;
    b->a2 = data.a2 / data.a0;

    /* initial sample */
    b->x1 = b->x2 = 0;
    b->y1 = b->y2 = 0;

    return 0;
}

// vim: et:sts=4:sw=4:cino=(0
