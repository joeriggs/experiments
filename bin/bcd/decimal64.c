/* This is an implementation of the decimal64 data representation.
 *
 * This implementation uses the "decimal representation method", which
 * uses "densely packed decimal" to store 5 groups of 3 digits, and it stores
 * the 4 most significant bits of the coefficient and the 2 most significant
 * bits of the exponent in a 5-bit combination.
 *
 * Refer to http://en.wikipedia.org/wiki/Decimal64_floating-point_format for a
 * description of the decimal64 representation.
 *
 * DPD<->BCD mapping works like this:
 *
 *   DPD encoded value -----------  BCD digits ---------------------------------
 *   b9 b8 b7 b6 b5 b4 b3 b2 b1 b0  d2   d1   d0    Values encoded   Description
 *   a  b  c  d  e  f  0  g  h  i   0abc 0def 0ghi  (0–7)(0–7)(0–7)  3 sm
 *   a  b  c  d  e  f  1  0  0  i   0abc 0def 100i  (0–7)(0–7)(8–9)  2 sm, 1 lg
 *   a  b  c  g  h  f  1  0  1  i   0abc 100f 0ghi  (0–7)(8–9)(0–7)  2 sm, 1 lg
 *   g  h  c  d  e  f  1  1  0  i   100c 0def 0ghi  (8–9)(0–7)(0–7)  2 sm, 1 lg
 *   g  h  c  0  0  f  1  1  1  i   100c 100f 0ghi  (8–9)(8–9)(0–7)  1 sm, 2 lg
 *   d  e  c  0  1  f  1  1  1  i   100c 0def 100i  (8–9)(0–7)(8–9)  1 sm, 2 lg
 *   a  b  c  1  0  f  1  1  1  i   0abc 100f 100i  (0–7)(8–9)(8–9)  1 sm, 2 lg
 *   x  x  c  1  1  f  1  1  1  i   100c 100f 100i  (8–9)(8–9)(8–9)  3 lg
 *
 * The combination field works like this:
 *
 *   Combo   Exponent   Coefficient
 *   Field   Bits 8/7      Digits
 *   00mmm      00          0mmm 
 *   01mmm      01          0mmm 
 *   10mmm      10          0mmm 
 *   1100m      00          100m 
 *   1101m      01          100m 
 *   1110m      10          100m 
 *   11110      --          ----
 *   11111      --          ----
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

#include "decimal64.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the packed representation of the decimal64 number. */
typedef union {
  uint64_t val;
  struct {
    uint64_t dpd_0      :10;
    uint64_t dpd_1      :10;
    uint64_t dpd_2      :10;
    uint64_t dpd_3      :10;
    uint64_t dpd_4      :10;
    uint64_t exponent   : 8; /* bits 7 - 0 of the exponent. */
    uint64_t combination: 5;
    uint64_t sign       : 1;
  } fields;
} decimal64_t;

/* This is the coefficient, expanded to the 16 BCD digit. */
typedef union {
  uint64_t val;
  struct {
    uint64_t bcd_0:12;
    uint64_t bcd_1:12;
    uint64_t bcd_2:12;
    uint64_t bcd_3:12;
    uint64_t bcd_4:12;
    uint64_t top:4;
  } bcd;
} coefficient_t;

/* This is the decimal64 class. */
struct decimal64 {

  /* The 16-digit coefficient. */
  coefficient_t coefficient;

  /* The 10-bit exponent. */
  uint16_t exponent;

  /* The 1-bit sign. */
  uint8_t  sign;
};

/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/* Display the contents of a decimal64 object.  This is only used for test and
 * debug.
 *
 * Input:
 *   this = A pointer to the decimal64 object.
 *
 * Output:
 *   true  = success.  The value was displayed.
 *   false = failure.  The value was NOT displayed.
 */
static bool
decimal64_disp(decimal64_t val)
{
  bool retcode = false;

  uint64_t u = val.val;
  uint64_t coeff = (u & 0x0003FFFFFFFFFFFFll);
  uint16_t exp   = val.fields.exponent;
  uint8_t  combo = val.fields.combination;
  uint8_t  sign  = val.fields.sign;
  coeff = coeff;
  exp   = exp;
  combo = combo;
  sign  = sign;
  DBG_PRINT("   val 0x%08llX.\n   bin ", u);

  int i;
  uint64_t b = 0x8000000000000000ll;
  for(i = 0; i < 64; i++)
  {
    DBG_PRINT("%d", (u & b) ? 1 : 0);
    b >>= 1;
    if((i == 0) || (i == 5) || (i == 13) || (i == 23) || (i == 33) || (i == 43) || (i == 53))
    {
      DBG_PRINT(" ");
    }
  }
  DBG_PRINT("\n");

  DBG_PRINT("       %1d 0x%02X %d 0x%013llX\n", sign, combo, exp, coeff);
  retcode = true;

  return retcode;
}

/* Expand a Densely Packed Decimal (DPD) into 3 BCD digits.
 *
 * Input:
 *   dpd  = The 10-bit DPD to convert to BCD.
 *
 *   bcd  = The address to store the BCD in.
 *
 * Output:
 *   true  = success.  The DPD was expanded.
 *   false = failure.  The DPD was NOT expanded.
 */
static bool
decimal64_dpd2bcd(uint16_t   dpd,
                  uint16_t  *bcd)
{
  bool retcode = false;

  if(bcd != (uint16_t *) 0)
  {
    uint8_t d0, d1, d2;
    d0 = d1 = d2 = 0;

    *bcd = 0;

    if((dpd & 0b0000001000) == 0b0000000000)
    {
      d0 = 0b0000 | ((dpd >> 7) & 0b0111);
      d1 = 0b0000 | ((dpd >> 4) & 0b0111);
      d2 = 0b0000 | ((dpd >> 0) & 0b0111);
      retcode = true;
    }
    else if((dpd & 0b0000001110) == 0b0000001000)
    {
      d0 = 0b0000 | ((dpd >> 7) & 0b0111);
      d1 = 0b0000 | ((dpd >> 4) & 0b0111);
      d2 = 0b1000 | ((dpd >> 0) & 0b0001);
      retcode = true;
    }
    else if((dpd & 0b0000001110) == 0b0000001010)
    {
      d0 = 0b0000 | ((dpd >> 7) & 0b0111);
      d1 = 0b1000 | ((dpd >> 4) & 0b0001);
      d2 = 0b0000 | ((dpd >> 4) & 0b0110) | ((dpd >> 0) & 0b0001);
      retcode = true;
    }
    else if((dpd & 0b0000001110) == 0b0000001100)
    {
      d0 = 0b1000 | ((dpd >> 7) & 0b0001);
      d1 = 0b0000 | ((dpd >> 4) & 0b0111);
      d2 = 0b0000 | ((dpd >> 7) & 0b0110) | ((dpd >> 0) & 0b0001);
      retcode = true;
    }
    else if((dpd & 0b0001101110) == 0b0000001110)
    {
      d0 = 0b1000 | ((dpd >> 7) & 0b0001);
      d1 = 0b1000 | ((dpd >> 4) & 0b0001);
      d2 = 0b0000 | ((dpd >> 7) & 0b0110) | ((dpd >> 0) & 0b0001);
      retcode = true;
    }
    else if((dpd & 0b0001101110) == 0b0000101110)
    {
      d0 = 0b1000 | ((dpd >> 7) & 0b0001);
      d1 = 0b0000 | ((dpd >> 6) & 0b0111) | ((dpd >> 4) & 0b0001);
      d2 = 0b1000 | ((dpd >> 0) & 0b0001);
      retcode = true;
    }
    else if((dpd & 0b0001101110) == 0b0001001110)
    {
      d0 = 0b0000 | ((dpd >> 7) & 0b0111);
      d1 = 0b1000 | ((dpd >> 4) & 0b0001);
      d2 = 0b1000 | ((dpd >> 0) & 0b0001);
      retcode = true;
    }
    else if((dpd & 0b0001101110) == 0b0001101110)
    {
      d0 = 0b1000 | ((dpd >> 7) & 0b0001);
      d1 = 0b1000 | ((dpd >> 4) & 0b0001);
      d2 = 0b1000 | ((dpd >> 0) & 0b0001);
      retcode = true;
    }

    /* Double check to see if things look correct. */
    if(retcode == true)
    {
      if( (d0 > 9) || (d1 > 9) || (d2 > 9) )
      {
        retcode = false;
      }
      /* Assemble the BCD digits. */
      else
      {
        *bcd = (d0 << 8) | (d1 << 4) | d2;
        DBG_PRINT("d0 %d (%X): d1 %d (%X): d2 %d (%X).  bcd %3X.\n",
                   d0, d0, d1, d1, d2, d2, *bcd);
      }
    }
  }

  return retcode;
}

/* Import a decimal64 value into this object.
 *
 * Input:
 *   this = A pointer to the decimal64 object.
 *
 *   src  = The decimal64 value to import.
 *
 * Output:
 *   true  = success.  The decimal64 value is imported.
 *   false = failure.  The decimal64 value is NOT imported.
 */
static bool
decimal64_import(decimal64 *this,
                 decimal64_t src)
{
  bool retcode = false;

  if(this != (decimal64 *) 0)
  {
    this->coefficient.val = 0;
    this->exponent        = src.fields.exponent;

    /* Convert the 5 Densely Packed Decimal (DPD) values into BCD digits. */
    do {
      uint16_t bcd;

      retcode = decimal64_dpd2bcd(src.fields.dpd_0, &bcd);
      if(retcode == false) break;
      this->coefficient.bcd.bcd_0 = bcd;

      retcode = decimal64_dpd2bcd(src.fields.dpd_1, &bcd);
      if(retcode == false) break;
      this->coefficient.bcd.bcd_1 = bcd;

      retcode = decimal64_dpd2bcd(src.fields.dpd_2, &bcd);
      if(retcode == false) break;
      this->coefficient.bcd.bcd_2 = bcd;

      retcode = decimal64_dpd2bcd(src.fields.dpd_3, &bcd);
      if(retcode == false) break;
      this->coefficient.bcd.bcd_3 = bcd;

      retcode = decimal64_dpd2bcd(src.fields.dpd_4, &bcd);
      if(retcode == false) break;
      this->coefficient.bcd.bcd_4 = bcd;
    } while(0);

    if(retcode == true)
    {
      /* Conver the combination field. */
      uint64_t combo = src.fields.combination;
      switch((combo >> 3) & 0b11)
      {
      case 0b00:
        /* Exponent bits 8/7 = 00.
         * Coefficient bit  56    = 0.
         *             bits 55-53 = combo bits 2-0. */
        this->coefficient.bcd.top |= (combo & 0b00111);
        break;

      case 0b01:
        /* Exponent bits 8/7 = 01.
         * Coefficient bit  56    = 0.
         *             bits 55-53 = combo bits 2-0. */
        this->exponent            |= 0b0100000000;
        this->coefficient.bcd.top |= (combo & 0b00111);
        break;

      case 0b10:
        /* Exponent bits 8/7 = 10.
         * Coefficient bit  56    = 0.
         *             bits 55-53 = combo bits 2-0. */
        this->exponent            |= 0b1000000000;
        this->coefficient.bcd.top |= (combo & 0b00111);
        break;

      case 0b11:
        switch((combo >> 1) & 0b11)
        {
        case 0b00:
        case 0b01:
        case 0b10:
          /* Exponent bits 8/7 = combo bits 2/1.
           * Coefficient bit  56-54 = 100.
           *             bits 53    = combo bit 0. */
          this->exponent            |= ((combo & 0b00110) << 6);
          this->coefficient.bcd.top |=                  0b1000;
          this->coefficient.bcd.top |= ((combo & 0b00001) << 3);
          break;

        case 0b11:
          if(combo & 0b00000)
          {
            /* +/- Infinity. */
          }
          else
          {
            /* NaN. */
          }
          break;
        }
      }
      DBG_PRINT("  %1d %03X %016llX\n",
                this->sign, this->exponent, this->coefficient.val);
    }
  }

  return retcode;
}

/* Compress 3 BCD digits down into a 10-bit DPD.
 *
 * Input:
 *   bcd  = The 10 bit BCD to convert to DPD.
 *
 *   dpd  = The address to store the DPD in.
 *
 * Output:
 *   true  = success.  The DPD was expanded.
 *   false = failure.  The DPD was NOT expanded.
 */
static bool
decimal64_bcd2dpd(uint16_t  bcd,
                  uint16_t *dpd)
{
  bool retcode = false;

  if(dpd != (uint16_t *) 0)
  {
    uint16_t d0_3lsb =  (bcd >> 0) & 0x07;
    uint16_t d0_1lsb =  (bcd >> 0) & 0x01;
    uint16_t d1_3lsb =  (bcd >> 4) & 0x07;
    uint16_t d1_1lsb =  (bcd >> 4) & 0x01;
    uint16_t d2_3lsb =  (bcd >> 8) & 0x07;
    uint16_t d2_1lsb =  (bcd >> 8) & 0x01;

    uint16_t d0, d1, d2, mask;

    uint16_t msb_bits = (bcd & 0x0888);
    switch(msb_bits)
    {
    case 0x0000:
      d0 = d0_3lsb << 0;
      d1 = d1_3lsb << 4;
      d2 = d2_3lsb << 7;
      mask = 0x000;
      break;

    case 0x0008:
      d0 = d0_1lsb << 0;
      d1 = d1_3lsb << 4;
      d2 = d2_3lsb << 7;
      mask = 0x008;
      break;

    case 0x0080:
      d0 = ((d0_3lsb & 0b0110) << 4) | d0_1lsb;
      d1 = d1_1lsb << 4;
      d2 = d2_3lsb << 7;
      mask = 0x00A;
      break;

    case 0x0800:
      d0 = ((d0_3lsb & 0b0110) << 7) | d0_1lsb;
      d1 = d1_3lsb << 4;
      d2 = d2_1lsb << 7;
      mask = 0x00C;
      break;

    case 0x0880:
      d0 = ((d0_3lsb & 0b0110) << 7) | d0_1lsb;
      d1 = d1_1lsb << 4;
      d2 = d2_1lsb << 7;
      mask = 0x00E;
      break;

    case 0x0808:
      d0 = d0_1lsb << 0;
      d1 = ((d2_3lsb & 0b0110) << 7) | (d1_1lsb << 4);
      d2 = d2_1lsb << 7;
      mask = 0x02E;
      break;

    case 0x0088:
      d0 = d0_1lsb << 0;
      d1 = d1_1lsb << 4;
      d2 = d2_3lsb << 7;
      mask = 0x04E;
      break;

    case 0x0888:
      d0 = d0_1lsb << 0;
      d1 = d1_1lsb << 4;
      d2 = d2_1lsb << 7;
      mask = 0x06E;
      break;
    }

    *dpd = (d0 | d1 | d2 | mask);
    retcode = true;
  }

  return retcode;
}

/* Export this into a decimal64.
 *
 * Input:
 *   this = A pointer to the decimal64 object.
 *
 *   dst  = A pointer to a 64-bit variable that will receive the decimal64 val.
 *
 * Output:
 *   true  = success.  *dst contains the decimal64 value.
 *   false = failure.  *dst is undefined.
 */
static bool
decimal64_export(decimal64   *this,
                 decimal64_t *dst)
{
  bool retcode = false;

  if( (this != (decimal64 *) 0) && (dst != (decimal64_t *) 0) )
  {
    coefficient_t c = this->coefficient;
    uint16_t      e = this->exponent;
    uint8_t       s = this->sign;

    dst->val = 0ll;

    /* Convert the 5 Densely Packed Decimal (DPD) values into BCD digits. */
    do {
      uint16_t dpd;

      retcode = decimal64_bcd2dpd(c.bcd.bcd_0, &dpd);
      if(retcode == false) break;
      dst->fields.dpd_0 = dpd;

      retcode = decimal64_bcd2dpd(c.bcd.bcd_1, &dpd);
      if(retcode == false) break;
      dst->fields.dpd_1 = dpd;

      retcode = decimal64_bcd2dpd(c.bcd.bcd_2, &dpd);
      if(retcode == false) break;
      dst->fields.dpd_2 = dpd;

      retcode = decimal64_bcd2dpd(c.bcd.bcd_3, &dpd);
      if(retcode == false) break;
      dst->fields.dpd_3 = dpd;

      retcode = decimal64_bcd2dpd(c.bcd.bcd_4, &dpd);
      if(retcode == false) break;
      dst->fields.dpd_4 = dpd;
    } while(0);

    /* Save the 8 LSBs of the exponent. */
    dst->fields.exponent = e & 0xFF;

    /* Create the combination field. */
    uint16_t e_2msb = ((e & 0x300) >> 8);
    if(c.bcd.top & 0b1000)
    {
      dst->fields.combination = 0b11000 | (e_2msb << 1) | ((c.bcd.top & 0b1000) >> 3);
    }
    else
    {
      dst->fields.combination = 0b01000 | (e_2msb << 3) | (c.bcd.top & 0b0111);
    }

    /* Put the sign on the front end. */
    dst->fields.sign = s;
  }

  return retcode;
}

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Create a new decimal64 object.  This object can be used to access the decimal64 class.
 *
 * Input:
 *   N/A.
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.
 */
decimal64 *
decimal64_new(void)
{
  decimal64 *this = (decimal64 *) 0;

  /* Initialize. */
  if((this = (decimal64 *) malloc(sizeof(*this))) != (decimal64 *) 0)
  {
  }

  return this;
}

/* Delete an decimal64 object that was created by decimal64_new().
 *
 * Input:
 *   this = A pointer to the decimal64 object.
 *
 * Output:
 *   true  = success.  The object is deleted.
 *   false = failure.
 */
bool
decimal64_delete(decimal64 *this)
{
  bool retcode = false;

  if(this != (decimal64 *) 0)
  {
    free(this);
    retcode = true;
  }

  return retcode;
}

/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#if defined(TEST)
bool
decimal64_test(void)
{
  bool retcode = true;

  /* Here are some problems to test against. */
  typedef struct decimal64_test {
    decimal64_t val;
  } decimal64_test;
  decimal64_test tests[] = {
    { .val.val = 0x22380000534B9C1Ell }, //        123456789
    { .val.val = 0x2A0A6828E56F3CA3ll }, //             2468.123456789123
    { .val.val = 0x263934B9C1E28E56ll }, // 1234567890123456
    { .val.val = 0x25F934B9C1E28E56ll }, //                0.1234567890123456
  };
  size_t tests_size = (sizeof(tests) / sizeof(decimal64_test));

  int x;
  for(x = 0; x < tests_size; x++)
  {
    decimal64_test *t = &tests[x];

    decimal64 *obj;
    obj = decimal64_new();
    retcode = (obj != (decimal64 *) 0) ? true : false;
    if(retcode == true)
    {
      printf("bef %016llX.\n", t->val.val);
      if((retcode = decimal64_disp(t->val))        != true) break;
      if((retcode = decimal64_import(obj, t->val)) != true) break;

      decimal64_t d;
      if((retcode = decimal64_export(obj, &d))     != true) break;
      if((retcode = decimal64_disp(d))             != true) break;
      DBG_PRINT("aft %016llX.\n", d.val);
      if((retcode = (t->val.val == d.val))         != true) break;
    }
  }

  return retcode;
}
#endif // TEST
