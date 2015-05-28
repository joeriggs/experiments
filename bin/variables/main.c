/* This program creates several integer and floating point variables and then
 * displays their contents on the console.  The purpose is to see how they are
 * stored in memory and to look at their min/max values. */

#include <stdint.h>
#include <stdio.h>

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
    double *d1 = (double *) data;
    printf("  dec: %5.40f\n", *d1);

    uint64_t *ui = (uint64_t *) data;
    printf("  hex: %08llX\n", *ui);
  }
  else if(data_size == sizeof(uint32_t))
  {
    float *f1 = (float *) data;
    printf("  dec: %5.40f\n", *f1);

    uint32_t *ui = (uint32_t *) data;
    printf("  hex: %04X\n", *ui);
  }

  disp_bin(data, data_size);
}

int main(int argc, char **argv)
{
  printf("Examine integer and floating point variables.\n");

  /* Initialize an entire double at one time. */
  double d1 = 123.456;
  disp_hex("d1", &d1, sizeof(d1));

  /* Add the decimal values one at a time. */
  int64_t i2 = 123;
  double d2 = i2;
  double multiplier = 0.10;
  unsigned char vals[] = { 4, 5, 6 };
  int i;
  for(i = 0; i < sizeof(vals); i++)
  {
    unsigned char c = vals[i];
    double d = c;
    d *= multiplier;
    d2 += d;
    multiplier /= 10.0;
  }
  disp_hex("d2", &d2, sizeof(d2));

  /* Try a regular float. */
  float f1 = 123.456;
  disp_hex("f1", &f1, sizeof(f1));

  return 0;
}

