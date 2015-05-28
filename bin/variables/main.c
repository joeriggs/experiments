/* This program creates several integer and floating point variables and then
 * displays their contents on the console.  The purpose is to see how they are
 * stored in memory and to look at their min/max values. */

#include <stdint.h>
#include <stdio.h>

typedef union {
  float    f;
  uint32_t u;
  struct {
    uint32_t fraction:23;
    uint32_t exponent:8;
    uint32_t sign:1;
  } ieee;
} float_t;

typedef union {
  double   d;
  uint64_t u;
  struct {
    uint64_t fraction2:32;
    uint64_t fraction1:20;
    uint64_t exponent:11;
    uint64_t sign:1;
  } ieee;
} double_t;

/* Binary output */
static void
disp_bin(void  *data,
         size_t data_size)
{
  printf("  bin: ");
  int i;
  for(i = 0; i < data_size; i++)
  {
    unsigned char *d = (unsigned char *) data;
    d += (data_size - i - 1);
    printf("%d%d%d%d%d%d%d%d ",
           ((*d & 0x80) >> 7), ((*d & 0x40) >> 6), ((*d & 0x20) >> 5), ((*d & 0x10) >> 4),
           ((*d & 0x08) >> 3), ((*d & 0x04) >> 2), ((*d & 0x02) >> 1), ((*d & 0x01) >> 0));
  }
  printf("\n");
}

static void
disp_hex(const char *data_name,
         void       *data,
         size_t      data_size)
{
  printf("\n%s: size %d:\n", data_name, data_size);

  /* Hex output. */
  if(data_size == sizeof(uint64_t))
  {
    double_t *d = (double_t *) data;
    printf("  d->d             = %f\n",   d->d);
    printf("  d->u             = %llX\n", d->u);
    printf("  d->ieee.sign     = %X\n",   d->ieee.sign);
    printf("  d->ieee.exponent = %X\n",   d->ieee.exponent);
    printf("  d->ieee.fraction = %X%X\n", d->ieee.fraction1, d->ieee.fraction2);
  }
  else if(data_size == sizeof(uint32_t))
  {
    float_t *f = (float_t *) data;
    printf("  f->f             = %f\n", f->f);
    printf("  f->u             = %X\n", f->u);
    printf("  f->ieee.sign     = %X\n", f->ieee.sign);
    printf("  f->ieee.exponent = %X\n", f->ieee.exponent);
    printf("  f->ieee.fraction = %X\n", f->ieee.fraction);

    /* Convert back to decimal. */
    float new_val = 0.0;
    int exp = f->ieee.exponent - 127;
    uint32_t frac = (1 << 23) | f->ieee.fraction;
    int i = 0x00800000;
    int first_val = 1;
    printf("  re-calc: ");
    while(i > 0)
    {
      if(frac & i)
      {
        if(first_val == 0)
        {
          printf(" + ");
        }
        first_val = 0;

        if(exp >= 0)
        {
          printf("%d", (1 << exp));
          new_val += (1 << exp);
        }
        else
        {
          printf("1/%d", (1 << (0 - exp)));
          new_val += (1.0 / (1 << (0 - exp)));
        }
      }
      i >>= 1;
      exp--;
    }
    printf("\n");
    if(f->ieee.sign == 1)
    {
      new_val = 0.0 - new_val;
    }
    printf("  re-calc'ed val: %f\n", new_val);
  }

  disp_bin(data, data_size);
}

int main(int argc, char **argv)
{
  printf("Examine integer and floating point variables.\n");

  /* A positive value. */
  double d1 = 123.456;
  disp_hex("d1", &d1, sizeof(d1));

  /* A negative value. */
  double d2 = -14.57;
  disp_hex("d2", &d2, sizeof(d2));

  /* Try a regular float. */
  float f1 = 123.456;
  disp_hex("f1", &f1, sizeof(f1));

  /* And a negative float. */
  float f2 = -14.57;
  disp_hex("f2", &f2, sizeof(f2));

  return 0;
}

