#include "math/biquad.h"

#include <stdlib.h>
#include <errno.h>
#include <math.h>

struct biquad {
    smp_t a0, a1, a2;
    smp_t b0, b1, b2;
    smp_t x1, x2, y1, y2;
};

typedef void (* filter_fn)(biquad_t *, smp_t, smp_t, smp_t, smp_t, smp_t);
static filter_fn filters[LAST_FILTER];

int
biquad_compute(biquad_t *b, filter_t *f)
{
    filter_fn filter = filters[f->type];
    if (!filter)
        return -EINVAL;

    smp_t A     = pow(10, f->gain / 40);
    smp_t omega = 2 * M_PI * f->fc / f->fs;
    smp_t sn    = sin(omega);
    smp_t cs    = cos(omega);
    smp_t alpha = sn * sinh(M_LN2 / 2 * f->bw * omega / sn);
    smp_t beta  = sqrt(A + A);

    /* apply the filter */
    filter(b, A, sn, cs, alpha, beta);

    /* precompute the coefficients */
    b->a1 /= b->a0;
    b->a2 /= b->a0;
    b->b0 /= b->a0;
    b->b1 /= b->a0;
    b->b2 /= b->a0;

    /* initial sample */
    b->x1 = b->x2 = 0;
    b->y1 = b->y2 = 0;

    return 0;
}

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

/* FILTER IMPLEMENTATIONS {{{ */
#define UNUSED(x) (void)(x)

void
low_pass(biquad_t *b, smp_t A, smp_t sn, smp_t cs, smp_t alpha, smp_t beta)
{
    UNUSED(A);
    UNUSED(sn);
    UNUSED(beta);

    b->b0 = (1 - cs) / 2;
    b->b1 =  1 - cs;
    b->b2 =  b->b0;
    b->a0 =  1 + alpha;
    b->a1 = -2 * cs;
    b->a2 =  1 - alpha;
}

void
high_pass(biquad_t *b, smp_t A, smp_t sn, smp_t cs, smp_t alpha, smp_t beta)
{
    UNUSED(A);
    UNUSED(sn);
    UNUSED(beta);

    b->b0 = (1 + cs) / 2;
    b->b1 = -1 - cs;
    b->b2 =  b->b0;
    b->a0 =  1 + alpha;
    b->a1 = -2 * cs;
    b->a2 =  1 - alpha;
}

void
band_pass(biquad_t *b, smp_t A, smp_t sn, smp_t cs, smp_t alpha, smp_t beta)
{
    UNUSED(A);
    UNUSED(sn);
    UNUSED(beta);

    b->b0 =  alpha;
    b->b1 =  0;
    b->b2 = -alpha;
    b->a0 =  1 + alpha;
    b->a1 = -2 * cs;
    b->a2 =  1 - alpha;
}

void
notch(biquad_t *b, smp_t A, smp_t sn, smp_t cs, smp_t alpha, smp_t beta)
{
    UNUSED(A);
    UNUSED(sn);
    UNUSED(beta);

    b->b0 =  1;
    b->b1 = -2 * cs;
    b->b2 =  1;
    b->a0 =  1 + alpha;
    b->a1 =  b->b1;
    b->a2 =  1 - alpha;
}

void
peaking_band(biquad_t *b, smp_t A, smp_t sn, smp_t cs, smp_t alpha, smp_t beta)
{
    UNUSED(sn);
    UNUSED(beta);

    b->b0 =  1 + alpha * A;
    b->b1 = -2 * cs;
    b->b2 =  1 - alpha * A;
    b->a0 =  1 + alpha / A;
    b->a1 =  b->b1;
    b->a2 =  1 - alpha / A;
}

void
low_shelf(biquad_t *b, smp_t A, smp_t sn, smp_t cs, smp_t alpha, smp_t beta)
{
    UNUSED(alpha);

    b->b0 =  A * (A + 1 - (A - 1) * cs + beta * sn);
    b->b1 =  2 * A * (A - 1 - (A + 1) * cs);
    b->b2 =  A * (A + 1 - (A - 1) * cs - beta * sn);
    b->a0 =  A + 1 + (A - 1) * cs + beta * sn;
    b->a1 = -2 * (A - 1 + (A + 1) * cs);
    b->a2 =  a->a0;
}

void
high_shelf(biquad_t *b, smp_t A, smp_t sn, smp_t cs, smp_t alpha, smp_t beta)
{
    UNUSED(alpha);

    b->b0 =  A * (A + 1 + (A - 1) * cs + beta * sn);
    b->b1 = -2 * A * (A - 1 + (A + 1) * cs);
    b->b2 =  A * (A + 1 + (A - 1) * cs - beta * sn);
    b->a0 =  A + 1 - (A - 1) * cs + beta * sn;
    b->a1 =  2 * (A - 1 - (A + 1) * cs);
    b->a2 =  a->a0;
}

static filter_fn filters[LAST_FILTER] = {
    [FILTER_LOW_PASS]     = low_pass,
    [FILTER_HIGH_PASS]    = high_pass,
    [FILTER_BAND_PASS]    = band_pass,
    [FILTER_NOTCH]        = notch,
    [FILTER_PEAKING_BAND] = peaking_band,
    [FILTER_LOW_SHELF]    = low_shelf,
    [FILTER_HIGH_SHELL]   = high_shelf,
};
/* }}} */

// vim: et:sts=4:sw=4:cino=(0
