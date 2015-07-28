/* This is a BCD (Binary Coded Decimal) implementation.  We need to be able to
 * do decimal math (float and double won't suffice), so this ADT provides that
 * capability.
 *
 * A good explanation of BCD addition and subtraction can be found at:
 *   http://homepage.cs.uiowa.edu/~jones/bcd/bcd.html
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "bcd.h"
#include "fp_exp.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define BCD_DBG_STR_TO_DECIMAL       0x0001
#define BCD_DBG_ADD_HALF_WIDTH       0x0002
#define BCD_DBG_SIGNIFICAND_ADD      0x0004
#define BCD_DBG_TENS_COMPLEMENT      0x0008
#define BCD_DBG_MAKE_EXPONENTS_EQUAL 0x0010
#define BCD_DBG_OP_ADD               0x0100
#define BCD_DBG_OP_SUB               0x0200
#define BCD_DBG_OP_MUL               0x0400
#define BCD_DBG_OP_DIV               0x0800
#define BCD_DBG_ADD_CHAR             0x1000
#define BCD_DBG_TO_STR               0x2000

#define BCD_DBG_PRINT_FLAGS (BCD_DBG_OP_MUL | BCD_DBG_TO_STR)
#define BCD_PRINT(FLAG, argc...) { if(FLAG & BCD_DBG_PRINT_FLAGS) { DBG_PRINT(argc); } }

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* The basic data element that stores the BCD digits internally. */
typedef uint32_t significand_section_t;

/* The data element that is passed to bcd_add_half_width().  It allows an
 * addition operation to carry at the top end. */
typedef uint64_t significand_large_section_t;

/* The number of digits that can be stored in a single data element. */
#define SIGNIFICAND_DIGITS_PER_SECTION (sizeof(significand_section_t) * 2)

/* This is the number of digits that the user can push into a bcd object via
 * bcd_add_char(). */
#define BCD_NUM_DIGITS 16 // Must be a multiple of 8

/* This is the number of digits that we work with internally.  It gives us a
 * lot of extra precision, thus allowing us to do things like:
 * - Perform repeated operations like exponentiation.
 * - Perform rounding on results.
 */
#define BCD_NUM_DIGITS_INTERNAL (BCD_NUM_DIGITS * 2)

/* The number of data elements required to hold all of the digits. */
#define SIGNIFICAND_SECTIONS_INTERNAL (BCD_NUM_DIGITS_INTERNAL / SIGNIFICAND_DIGITS_PER_SECTION)

/* The definition of the significand that is located in each bcd object. */
typedef struct { significand_section_t s[SIGNIFICAND_SECTIONS_INTERNAL]; } significand_t;

#define SIGNIFICAND_ADD_HALF_VAL1              0x066666666ll
#define SIGNIFICAND_ADD_HALF_VAL2              0x111111110ll
#define SIGNIFICAND_SECTION_MASK                0xFFFFFFFF
#define SIGNIFICAND_SECT_TENS_COMPLEMENT_VAL    0x99999999

/* This is the bcd class. */
struct bcd {

  /* This is the significand.  Each nybble equals one decimal digit.*/
  significand_t significand;

  /* This is the exponent.
   * - It's >= 0 if (|number| >= 1).
   * - It's < 0 if (|number| < 1).
   */
  int16_t exponent;

  /* This is the sign.
   * - true = negative.
   * - false = positive.
   */
  uint8_t sign;

  /* If we're adding one character at a time, these are used to help us know
   * where we are. */
  int char_count;
  bool got_decimal_point;
};
  
/******************************************************************************
 ******************************** PRIMITIVES **********************************
 *****************************************************************************/

/* Given a significand_section_t, return the digit at the specified offset.  The
 * offset is relative to the left-hand side of the section.  For example, if the
 * section currently contains the number "98765432" and you request offset
 * 2, you will get the number 7.
 *
 * Input:
 *   section = The section.
 *
 *   offset  = The offset into the section.
 *
 * Output:
 *   If success, returns the digit.
 *   If failure, returns 0xF.
 */
static uint8_t
bcd_sect_get_digit(significand_section_t section,
                   int                   offset)
{
  uint8_t retval = 0xF;

  if(offset < SIGNIFICAND_DIGITS_PER_SECTION)
  {
    int shift = (((SIGNIFICAND_DIGITS_PER_SECTION - 1) - offset) * 4);
    uint32_t tmp = ((section >> shift) & 0xF);
    retval = tmp;
  }

  return retval;
}

/* Given a significand_section_t, set the digit at the specified offset.  The
 * offset is relative to the left-hand side of the section.  For example, if the
 * section currently contains the number "98765432" and you want to set the
 * value at offset 2 to 0, the result will be "98065432".
 *
 * Input:
 *   section = A pointer to the section.
 *
 *   offset  = The offset into the section.
 *
 *   value   = The value to set in section[offset].
 *
 * Output:
 *   Return true  if success.  The value is set.
 *   Return false if failure.
 */
static bool
bcd_sect_set_digit(significand_section_t *section,
                   int                    offset,
                   uint8_t                value)
{
  bool retcode = false;

  if(offset < SIGNIFICAND_DIGITS_PER_SECTION)
  {
    /* Calculate the number of bits positions we need to shift value (to the
     * left) in order to line it up with section[offset]. */
    int shift = (((SIGNIFICAND_DIGITS_PER_SECTION - 1) - offset) * 4);

    /* Clear the current value from section[offset]. */
    significand_section_t mask1 = SIGNIFICAND_SECTION_MASK;
    significand_section_t mask2 = (0xF << shift);
    mask1 ^= mask2;
    *section &= mask1;

    /* Now insert value. */
    significand_section_t shifted_val = ((significand_section_t) value) << shift;
    *section |= shifted_val;

    retcode = true;
  }

  return retcode;
}

/* Given a significand, return the digit at the specified offset.  The offset
 * is relative to the left-hand side of the number.  For example, if the
 * significand currently contains the number "987654" and you request offset
 * 2, you will get the number 7.
 *
 * Input:
 *   significand = A pointer to the significand.
 *
 *   offset      = The offset into the significand.
 *
 * Output:
 *   If success, returns the digit.
 *   If failure, returns 0xF.
 */
static uint8_t
bcd_sig_get_digit(significand_t *significand,
                  int            offset)
{
  uint8_t retval = 0xF;

  if( (significand != (significand_t *) 0) && (offset < BCD_NUM_DIGITS_INTERNAL) )
  {
    int index = offset / SIGNIFICAND_DIGITS_PER_SECTION;
    int section_offset = (offset % 8);
    retval = bcd_sect_get_digit(significand->s[index], section_offset);
  }

  return retval;
}

/* Given a significand, set the digit at the specified offset.  The offset
 * is relative to the left-hand side of the section.  For example, if the number
 * currently contains the number "98765432" and you want to set the value at
 * offset 2 to 0, the result will be "98065432".
 *
 * Input:
 *   significand = A pointer to the significand.
 *
 *   offset      = The offset into the significand.
 *
 *   value       = The value to set in significand[offset].
 *
 * Output:
 *   Return true  if success.  The value is set.
 *   Return false if failure.
 */
static bool
bcd_sig_set_digit(significand_t *significand,
                  int            offset,
                  int            value)
{
  bool retcode = false;

  if( (significand != (significand_t *) 0) && (offset < BCD_NUM_DIGITS_INTERNAL) )
  {
    int index = offset / SIGNIFICAND_DIGITS_PER_SECTION;
    int section_offset = (offset % 8);
    retcode = bcd_sect_set_digit(&significand->s[index], section_offset, value);
  }

  return retcode;
}

/* Shift the significand left or right by the specified number of places.
 *
 * Input:
 *   sig   = A pointer to the significand.
 *
 *   shift = The number of places to shift.
 *           - A positive number indicates a right shift.
 *           - A negative number indicates a left shift.
 *           - Zero indicates no shift.
 *
 * Output:
 *   true  = success.  op and exp have been adjusted.
 *   false = failure.  The states of op and exp are undefined.
 */
static bool
bcd_shift_significand(significand_t *sig,
                      int16_t        shift)
{
  bool retcode = false;

  if(sig != (significand_t *) 0)
  {
    int i;

    /* Left shift starts at the beginning of the number. */
    if(shift < 0)
    {
      for(i = 0; i < BCD_NUM_DIGITS_INTERNAL; i++)
      {
        int src_offset = i - shift;
        int dst_offset = i;
        uint8_t c = (src_offset >= BCD_NUM_DIGITS_INTERNAL) ? 0 : bcd_sig_get_digit(sig, src_offset);
        if((retcode = bcd_sig_set_digit(sig, dst_offset, c)) != true)
        {
          break;
        }
      }
    }

    /* Right shift starts at the end. */
    else if(shift > 0)
    {
      for(i = (BCD_NUM_DIGITS_INTERNAL - 1); i >= 0; i--)
      {
        int src_offset = i - shift;
        int dst_offset = i;
        uint8_t c = (src_offset < 0) ? 0 : bcd_sig_get_digit(sig, src_offset);
        if((retcode = bcd_sig_set_digit(sig, dst_offset, c)) != true)
        {
          break;
        }
      }
    }
   

    /* No shift is okay too. */
    else
    {
      retcode = true;
    }
  }

  return retcode;
}

/* Initialize a significand to zero.
 *
 * Input:
 *   significand = A pointer to the significand.
 *
 * Output:
 *   true  = success.  The significand is now set to zero.
 *   false = failure.
 */
static bool
bcd_sig_initialize(significand_t *significand)
{
  bool retcode = false;

  if(significand != (significand_t *) 0)
  {
    int i;
    for(i = 0; i < SIGNIFICAND_SECTIONS_INTERNAL; i++)
    {
      significand->s[i] = 0;
    }
    retcode = true;
  }

  return retcode;
}

/* Check to see if a significand is zero.
 *
 * Input:
 *   significand = A pointer to the significand.
 *
 * Output:
 *   Returns true if it's zero.
 *   Returns false if it's not zero, or if significand is an invalid pointer.
 */
static bool
bcd_sig_is_zero(significand_t *significand)
{
  bool retcode = false;

  if(significand != (significand_t *) 0)
  {
    int i;
    for(i = 0; (i < SIGNIFICAND_SECTIONS_INTERNAL) && (significand->s[i] == 0); i++);
    if(i == SIGNIFICAND_SECTIONS_INTERNAL)
    {
      retcode = true;
    }
  }

  return retcode;
}

/* Make a copy of a significand.
 *
 * Input:
 *   src = A pointer to the original.
 *
 *   dst = A pointer to the copy.
 *
 * Output:
 *   true  = success.  *dst == *src.
 *   false = failure.
 */
static bool
bcd_sig_copy(significand_t *src,
             significand_t *dst)
{
  bool retcode = false;

  if( (src != (significand_t *) 0) && (dst != (significand_t *) 0) )
  {
    int i;
    for(i = 0; i < SIGNIFICAND_SECTIONS_INTERNAL; i++)
    {
      dst->s[i] = src->s[i];
    }
    retcode = true;
  }

  return retcode;
}

/* Compare 2 significands.
 *
 * Input:
 *   src      = A pointer to one of the significands.
 *
 *   src_mask = [optional] A bitmask to use against *src before the compare.
 *              The bitmask allows this method to compare part of *src against
 *              *dst.
 *
 *   dst      = A pointer to the other significand.
 *
 *   dst_mask = [optional] A bitmask to use against *dst before the compare.
 *              The bitmask allows this method to compare part of *dst against
 *              *src.
 *
 * Output:
 *   -1 if *src < *dst.
 *    0 if *src == *dst.
 *    1 if *src > *dst.
 */
static int
bcd_sig_cmp(significand_t *src,
            significand_t *src_mask,
            significand_t *dst,
            significand_t *dst_mask)
{
  int retval = 0;

  if( (src != (significand_t *) 0) && (dst != (significand_t *) 0) )
  {
    int i;
    for(i = 0; i < SIGNIFICAND_SECTIONS_INTERNAL; i++)
    {
      significand_section_t src_section = src->s[i];
      if(src_mask != (significand_t *) 0)
      {
        src_section &= src_mask->s[i];
      }

      significand_section_t dst_section = dst->s[i];
      if(dst_mask != (significand_t *) 0)
      {
        dst_section &= dst_mask->s[i];
      }

      if( src_section < dst_section) { retval = -1; break; }
      if( src_section > dst_section) { retval =  1; break; }
    }
  }

  return retval;
}

/* Count the number of significant digits in the significand.
 *
 * Input:
 *   significand = A pointer to the significand.
 *
 * Output:
 *   Returns the number of significant digits.
 *   If an error occurs, it returns -1.
 */
static int
bcd_sig_num_digits(significand_t *significand)
{
  int retval = -1;

  if(significand != (significand_t *) 0)
  {
    /* If the number is zero, then there are zero significant digits. */
    if(bcd_sig_is_zero(significand) == true)
    {
      retval = 0;
    }

    /* If the number is NOT zero, then we need to count the number of
     * insignificant digits. */
    else
    {
      int i;
      for(i = BCD_NUM_DIGITS_INTERNAL; i > 0; i--)
      {
        if(bcd_sig_get_digit(significand, (i - 1)) != 0)
        {
          retval = i;
          break;
        }
      }
    }
  }

  return retval;
}

#if defined(DEBUG)
/* This function will convert a significand_section_t to an ASCII string.  It
 * returns a pointer to the string.
 *
 * WARNING: This function will maintain a limited number of buffers that can
 *          hold a string.  The caller doesn't need to free the string after
 *          they use it, but they shouldn't hold onto it for a long time and
 *          expect it to remain the same.  The purpose of this member is to
 *          provide a string that can be used to debug/test purposes.  It's not
 *          production-quality code.  Use it as it was intended to be used!
 *
 * Input:
 *   section = The significand_section_t to convert.
 *
 * Output:
 *   If success, returns a pointer to a buffer that contains the string.
 *   If failure, returns a pointer to the string "UNKNOWN".
 */
static const char *
bcd_sig_section_to_str(significand_section_t section)
{
  char *retval = "UNKNOWN";

  /* This is a collection of buffers that is used for building strings.
   * NOTE: The size has to be a power of 2. */
  static char *sig_msgs[16] = { 0 };
  static char  sig_msgs_size = (sizeof(sig_msgs) / sizeof(sig_msgs[0]));
  static int   sig_msgs_index = 0;

  /* We allocate the pointer the first time we need it. */
  if(sig_msgs[sig_msgs_index] == (char *) 0)
  {
    sig_msgs[sig_msgs_index] = (char *) malloc(SIGNIFICAND_DIGITS_PER_SECTION + 1);
  }

  if(sig_msgs[sig_msgs_index] != (char *) 0)
  {
    retval = sig_msgs[sig_msgs_index];
    sig_msgs_index = ((sig_msgs_index + 1) % sig_msgs_size);

    /* Create the string now. */
    int i;
    for(i = 0; i < SIGNIFICAND_DIGITS_PER_SECTION; i++)
    {
      char c = bcd_sect_get_digit(section, i);
      retval[i] = (c > 9) ? (c + 0x37) : (c + 0x30);
    }
    retval[i] = 0;
  }

  return retval;
}

/* This function will convert a significand_t to an ASCII string.  It returns a
 * pointer to the string.
 *
 * WARNING: This function will maintain a limited number of buffers that can
 *          hold a string.  The caller doesn't need to free the string after
 *          they use it, but they shouldn't hold onto it for a long time and
 *          expect it to remain the same.  The purpose of this member is to
 *          provide a string that can be used to debug/test purposes.  It's not
 *          production-quality code.  Use it as it was intended to be used!
 *
 * Input:
 *   significand = The significand to convert.
 *
 * Output:
 *   If success, returns a pointer to a buffer that contains the string.
 *   If failure, returns a pointer to the string "UNKNOWN".
 */
static const char *
bcd_sig_to_str(significand_t *significand)
{
  char *retval = "UNKNOWN";

  /* This is a collection of buffers that is used for building strings.
   * NOTE: The size has to be a power of 2. */
  static char *sig_msgs[16] = { 0 };
  static char  sig_msgs_size = (sizeof(sig_msgs) / sizeof(sig_msgs[0]));
  static int   sig_msgs_index = 0;

  if(significand != (significand_t *) 0)
  {
    /* We allocate the pointer the first time we need it. */
    if(sig_msgs[sig_msgs_index] == (char *) 0)
    {
      sig_msgs[sig_msgs_index] = (char *) malloc(BCD_NUM_DIGITS_INTERNAL + 1);
    }

    if(sig_msgs[sig_msgs_index] != (char *) 0)
    {
      retval = sig_msgs[sig_msgs_index];
      sig_msgs_index = ((sig_msgs_index + 1) % sig_msgs_size);
      retval[0] = 0;

      /* Create the string now. */
      int i;
      for(i = 0; i < SIGNIFICAND_SECTIONS_INTERNAL; i++)
      {
        strcat(retval, bcd_sig_section_to_str(significand->s[i]));
      }
    }
  }

  return retval;
}
#endif // DEBUG

/* This is a utility function.  You pass it a significand/exponent/sign, and it
 * creates a Sxxx.xxx ASCII string for you.  It does nothing fancy beyond that.
 * You have to make sure you pass it a BCD number that will fit that format.
 *
 * If the number requires more than 16 digits, then this function will fail.  It
 * expects the number to fit in a regular decimal notation.
 *
 * Input:
 *   significand       = A pointer to the BCD digits.
 *
 *   exponent          = The exponent.  It tells us where to place the decimal
 *                       point.
 *
 *   char_count        = This is used in situations where the user is typing in
 *                       a number.  It tells us how many digits they have typed.
 *                       It's particularly useful for situations where they're
 *                       typing zeroes after the decimal point (ex, 0.00000).
 *                       We want to display all of the characters they've typed,
 *                       even if it means displaying a bunch of insignificant
 *                       zeroes.
 *
 *   got_decimal_point = true if we need to insert the decimal point.  You will
 *                       see this in situations where we're capturing the
 *                       keystrokes from the user and they hit the decimal
 *                       point key (even if there aren't any significant digits
 *                       after the decimal point).
 *
 *   sign              = Do we need a sign on the front?
 *
 *   buf               = The caller-supplied buffer to build the string in.
 *
 *   buf_size          = The size of buf.  Note that we must allow 1 byte for
 *                       the NULL terminator.
 *
 * Output:
 *   true  = success.  buf contains the string.  Note that it might be truncated
 *                     if buf is too small to hold the entire string.
 *   false = failure.  The contents of buf are undefined.
 */
static bool
bcd_to_str_decimal(significand_t *significand,
                   int16_t        exponent,
                   int            char_count,
                   bool           got_decimal_point,
                   uint8_t        sign,
                   char          *buf,
                   size_t         buf_size)
{
  bool retcode = false;

  /* We need a buffer and a number that is <= BCD_NUM_DIGITS long. */
  int16_t max_exp = (BCD_NUM_DIGITS - 1);
  int16_t min_exp = (0 - max_exp);
  if( (buf != (char *) 0) && (exponent <= max_exp) && (exponent >= min_exp) )
  {
    int buf_x = 0;

    do
    {
      BCD_PRINT(BCD_DBG_STR_TO_DECIMAL, "%s(): %s, %d, %d, %d.\n", __func__,
                bcd_sig_to_str(significand), exponent, got_decimal_point, sign);

      /* If it's a negative number insert the sign now.  Even -0 gets a sign. */
      if(sign != 0)
      {
        if(--buf_size == 0) { break; } else { buf[buf_x++] = '-'; }
      }

      /* If (|number| < 0), then we need to insert "0." and maybe significant
       * zeroes after the decimal point. */
      if(exponent < 0)
      {
        /* Insert the "0.". */
        if(--buf_size == 0) { break; } else { buf[buf_x++] = '0'; }
        if(--buf_size == 0) { break; } else { buf[buf_x++] = '.'; }

        /* Insert significant zeroes after the decimal point. */
        while(exponent < -1)
        {
          if(--buf_size == 0) { break; } else { buf[buf_x++] = '0'; }
          exponent++;
        }
      }

      /* How many digits are there?  The possibilities are in this order:
       *
       * 1. If (char_count > 0) (i.e. this was populated via bcd_add_char(),
       *    then (digit_count = char_count).
       *
       * 2. max((exponent + 1), (non-zero digits)).
       *    2A. (exponent + 1) is the minimum number of digits.  For example,
       *        significand = 2000000000, exponent = 4, num = "20,000".
       *    2B. (non-zero digits) are all displayed.  For example,
       *        significand = 1234560000, exponent = 0, num = "1.23456".
       */
      int digit_count = char_count;
      if(digit_count == 0)
      {
        /* Count the non-zero digits in the significand. */
        int i = min(bcd_sig_num_digits(significand), BCD_NUM_DIGITS);
        if(i == -1) break;

        digit_count = max(i, (exponent + 1));
      }

      int digit_position = 0;
      for( ; digit_count > 0; digit_count--)
      {
        char c = bcd_sig_get_digit(significand, digit_position++);

        /* Insert the digit. */
        if(--buf_size == 0) { break; } else { buf[buf_x++] = (c | 0x30); }

        /* Insert commas (base-1000). */
        if((exponent > 0) && ((exponent % 3) == 0))
        {
          if(--buf_size == 0) { break; } else { buf[buf_x++] = ','; }
        }

        /* Do we need a decimal point? */
        if((exponent-- == 0) && ((digit_count > 1) || (got_decimal_point == true)))
        {
          if(--buf_size == 0) { break; } else { buf[buf_x++] = '.'; }
        }
      }
    } while(0);

    buf[buf_x] = 0;
    retcode = true;
  }

  return retcode;
}

/* Perform a straight addition of 2 sections of a significand.  The caller
 * passes an extra large value to store the result so we don't have to worry
 * about overflow.  We simply add the 2 values and place the sum in the large
 * container.  If there was a carry above the size of a regular
 * significand_section_t, the caller will handle it.
 *
 * Input:
 *   val1   = One significand_section_t value.
 *
 *   val2   = One significand_section_t value.
 *
 *   dst    = A pointer to the place to store the result.  Note that the result
 *            might carry up into the upper-half of *dst.
 *
 * Output:
 *   true  = success.  *dst contains the sum.
 *   false = failure.  *dst is undefined.
 */
static bool
bcd_add_half_width(significand_section_t        val1,
                   significand_section_t        val2,
                   significand_large_section_t *dst)
{
  bool retcode = false;

  if(dst != (significand_large_section_t *) 0)
  {
    BCD_PRINT(BCD_DBG_ADD_HALF_WIDTH, "%s(%s %s)\n", __func__, bcd_sig_section_to_str(val1), bcd_sig_section_to_str(val2));
    significand_large_section_t t1 = val1 + SIGNIFICAND_ADD_HALF_VAL1;
    significand_large_section_t t2 = t1 + val2;
    significand_large_section_t t3 = t1 ^ val2;
    significand_large_section_t t4 = t2 ^ t3;
    significand_large_section_t t5 = ~t4 & SIGNIFICAND_ADD_HALF_VAL2;
    significand_large_section_t t6 = (t5 >> 2) | (t5 >> 3);
    *dst = t2 - t6;
    BCD_PRINT(BCD_DBG_ADD_HALF_WIDTH, "%s(): SUM %s:%s.\n", __func__,
              bcd_sig_section_to_str((*dst) >> (SIGNIFICAND_DIGITS_PER_SECTION * 4)),
              bcd_sig_section_to_str(*dst));

    retcode = true;
  }

  return retcode;
}

/* Perform a straight addition of 2 significands.  No exponents or signs are
 * involved.  That needs to be taken care of somewhere else.  This method does
 * nothing more than add the 2 significands and returns the result.
 *
 * Input:
 *   val1      = A pointer to one of the significands.
 *
 *   val2      = A pointer to one of the significands.
 *
 *   dst       = A pointer to the place to store the result.  Note that dst can
 *               point to val1 or val2.
 *
 *   carry     = [optional] A pointer to a variable that will be set to true if
 *                          a carry occurs "at the end" of the addition.
 *               NULL = Do NOT check for carry.  Only pass in a non-NULL if you
 *                      really want to check for carry.  It's a manual operation
 *                      in this function, so don't request it unless you need it.
 *               Examples of when carry is set or not set:
 *                   8 + 5 = 13 is what we're looking for.
 *                  35 + 7 = 42 is NOT what we're looking for.
 *
 *   overflow  = A pointer to a variable that will receive the overflow if the
 *               sum doesn't fit in *dst.
 *
 * Output:
 *   true  = success.  *dst and *overflow contain the sum.
 *   false = failure.  *dst and *overflow are undefined.
 */
static bool
bcd_significand_add(significand_t *val1,
                    significand_t *val2,
                    significand_t *dst,
                    bool          *carry,
                    uint8_t       *overflow)
{
  bool retcode = false;

  if( (    val1 != (significand_t *) 0) &&
      (    val2 != (significand_t *) 0) &&
      (     dst != (significand_t *) 0) &&
      (overflow != (uint8_t *) 0) )
  {
    BCD_PRINT(BCD_DBG_SIGNIFICAND_ADD, "%s(%s, %s)\n", __func__, bcd_sig_to_str(val1), bcd_sig_to_str(val2));

    do
    {
      /* If we're checking for carry, locate the high-order significant digit
       * before we add.  carry_digit is set to the highest non-zero digit in
       * val1 and val2. */
      int carry_digit = 0;
      if(carry != (bool *) 0)
      {
        for(carry_digit = 0; carry_digit < BCD_NUM_DIGITS; carry_digit++)
        {
          if( (bcd_sig_get_digit(val1, carry_digit) != 0) || (bcd_sig_get_digit(val2, carry_digit) != 0) )
          {
            break;
          }
        }
      }

      /* We'll change this if we encounter an actual overflow. */
      *overflow = 0;

      /* Loop through all of the significand_section_t chunks, starting with
       * the least significant. */
      int i;
      for(i = (SIGNIFICAND_SECTIONS_INTERNAL - 1); i >= 0; i--)
      {
        significand_large_section_t sum;

        /* Add the 2 sections. */
        if((retcode = bcd_add_half_width(val1->s[i], val2->s[i], &sum)) != true) { break; }
        dst->s[i] = (sum & SIGNIFICAND_SECTION_MASK);
        BCD_PRINT(BCD_DBG_SIGNIFICAND_ADD, "%s() LOOP: %s\n", __func__, bcd_sig_section_to_str(dst->s[i]));

        /* Handle carry. */
        if((sum >> (SIGNIFICAND_DIGITS_PER_SECTION * 4)) > 0)
        {
          if(i > 0)
          {
            uint8_t tmp_overflow;
            significand_t ovf = { .s = { 0 } };
            ovf.s[i - 1] = 1;
            if(bcd_significand_add(val1, &ovf, val1, 0, &tmp_overflow) != true) { break; }
            *overflow += tmp_overflow;
          }
          else
          {
            (*overflow)++;
          }
        }
      }

      BCD_PRINT(BCD_DBG_SIGNIFICAND_ADD, "%s() AFTER LOOP: %s: 0x%X\n", __func__, bcd_sig_to_str(dst), *overflow);

      /* If the caller requested a carry check, check now to see if there
       * was a carry at the end. */
      if(carry != (bool *) 0)
      {
        *carry = (carry_digit > 0) ? ((bcd_sig_get_digit(dst, (carry_digit - 1)) != 0ll) ? true : false) : 0;
      }

      BCD_PRINT(BCD_DBG_SIGNIFICAND_ADD, "%s() RESULT: %s: 0x%X\n", __func__, bcd_sig_to_str(dst), *overflow);
      retcode = true;
    } while(0);
  }

  return retcode;
}

/* Perform a 10's complement on a BCD significand.  This changes positives to
 * negatives and negatives to positives.
 *
 * Input:
 *   src  = A pointer to the significand.  We don't need the sign or exponent.
 *
 *   dst  = A pointer to the place to store the result.  Note that dst can
 *          point to src.
 *
 * Output:
 *   true  = success.  *dst contains the 10's complement of src.
 *   false = failure.  *dst is undefined.
 */
static bool
bcd_tens_complement(significand_t *src,
                    significand_t *dst)
{
  bool retcode = false;

  if(dst != (significand_t *) 0)
  {
    BCD_PRINT(BCD_DBG_TENS_COMPLEMENT, "%s(): src: %s.\n", __func__, bcd_sig_to_str(src));

    /* Do a 9's complement first. */
    int i;
    for(i = 0; i < SIGNIFICAND_SECTIONS_INTERNAL; i++)
    {
      dst->s[i] = SIGNIFICAND_SECT_TENS_COMPLEMENT_VAL - src->s[i];
    }

    /* Now add 1 to complete the 10's complement. */
    uint8_t overflow;
    significand_t one = { .s[SIGNIFICAND_SECTIONS_INTERNAL - 1] = 1 };
    retcode = bcd_significand_add(dst, &one, dst, NULL, &overflow);
    BCD_PRINT(BCD_DBG_TENS_COMPLEMENT, "%s(): dst: %s.\n", __func__, bcd_sig_to_str(dst));
  }

  return retcode;
}

/* Given 2 numbers, adjust the smaller one so that its exponent is the same as
 * the larger one.
 *
 * Input:
 *   op1  = A pointer to the 1st operand.
 *
 *   exp1 = A pointer to the exponent for the 1st operand.
 *
 *   op2  = A pointer to the 2nd operand.
 *
 *   exp2 = A pointer to the exponent for the 2nd operand.
 *
 * Output:
 *   true  = success.  They have been adjusted.
 *   false = failure.  The states of the 4 arguments is undefined.
 */
static bool
bcd_make_exponents_equal(significand_t *op1,
                         int16_t       *exp1,
                         significand_t *op2,
                         int16_t       *exp2)
{
  bool retcode = false;

  if((op1 != (significand_t *) 0) && (exp1 != (int16_t *) 0) &&
     (op2 != (significand_t *) 0) && (exp2 != (int16_t *) 0))
  {
    BCD_PRINT(BCD_DBG_MAKE_EXPONENTS_EQUAL,"%s(%p, %d, %p, %d).\n", __func__, op1, *exp1, op2, *exp2);

    significand_t *tgt_sig = (significand_t *) 0;
    int16_t       *tgt_exp = (int16_t *) 0;
    int shift = 0;
    if(*exp1 > *exp2)
    {
      BCD_PRINT(BCD_DBG_MAKE_EXPONENTS_EQUAL,"%s(): Adjusting op2 so its exponent equals the op1 exponent.\n", __func__);
      tgt_sig = op2;
      tgt_exp = exp2;
      shift = *exp1 - *exp2;
    }
    else if(*exp1 < *exp2)
    {
      BCD_PRINT(BCD_DBG_MAKE_EXPONENTS_EQUAL,"%s(): Adjusting op1 so its exponent equals the op2 exponent.\n", __func__);
      tgt_sig = op1;
      tgt_exp = exp1;
      shift = *exp2 - *exp1;
    }

    /* If we need to adjust, do it here. */
    if((tgt_sig != (significand_t *) 0) && (shift != 0))
    {
      *tgt_exp += shift;
      retcode = bcd_shift_significand(tgt_sig, shift);
    }
    else
    {
      retcode = true;
    }
  }

  return retcode;
}

/* Shift the significand to remove leading zeroes.
 *
 * Input:
 *   sig = A pointer to the significand.
 *
 *   exp = A pointer to the exponent for the significand.
 *
 * Output:
 *   true  = success.  sig and exp have been adjusted.
 *   false = failure.  The states of sig and exp are undefined.
 */
static bool
bcd_sig_remove_leading_zeroes(significand_t *sig,
                              int16_t       *exp)
{
  bool retcode = false;

  if((sig != (significand_t *) 0) && (exp != (int16_t *) 0))
  {
    /* Set the success retcode before we do the shift.  That way we'll be able
     * to detect failures. */
    retcode = true;

    while((bcd_sig_is_zero(sig) == false) && (bcd_sig_get_digit(sig, 0) == 0))
    {
      if((retcode = bcd_shift_significand(sig, -1)) != true)
      {
        break;
      }
      *exp = *exp - 1;
    }
  }

  return retcode;
}

/******************************************************************************
 ********************************* PUBLIC OPS *********************************
 *****************************************************************************/

/* This is the BCD addition function.
 *
 * Input:
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Addtion is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the sum.
 *   false = failure.
 */
bool
bcd_op_add(bcd *op1,
           bcd *op2)
{
  bool retcode = false;

  if((op1 != (bcd *) 0) && (op2 != (bcd *) 0))
  {
    do
    {
      significand_t *sig1 = &op1->significand;
      significand_t *sig2 = &op2->significand;
      BCD_PRINT(BCD_DBG_OP_ADD, "%s()           BEGIN: %s + %s\n", __func__, bcd_sig_to_str(sig1), bcd_sig_to_str(sig2));

      /* If the exponents aren't the same, adjust the smaller number up to the other. */
      if((retcode = bcd_make_exponents_equal(sig1, &op1->exponent, sig2, &op2->exponent)) != true) break;

      /* If either num is negative, do a 10's complement before the addition. */
      if(op1->sign == true) { if((retcode = bcd_tens_complement(sig1, sig1)) == false) break; }
      if(op2->sign == true) { if((retcode = bcd_tens_complement(sig2, sig2)) == false) break; }
      BCD_PRINT(BCD_DBG_OP_ADD, "%s() 10'S COMPLEMENT: %s, %s\n", __func__, bcd_sig_to_str(sig1), bcd_sig_to_str(sig2));

      uint8_t overflow;
      if((retcode = bcd_significand_add(sig1, sig2, sig1, NULL, &overflow)) != true) { break; }
      BCD_PRINT(BCD_DBG_OP_ADD, "%s():         RESULT: %s: overflow %d\n", __func__, bcd_sig_to_str(sig1), overflow);

      /* If exactly one of the operands is negative:
       * - If we have overflow, then the result is positive.
       * - If we have no overflow, then the result is negative.
       */
      if(op1->sign != op2->sign)
      {
        if(overflow == 0)
        {
          if((retcode = bcd_tens_complement(sig1, sig1)) == false) break;
        }
        op1->sign = (overflow == 0) ? true : false;
      }

      /* Otherwise the signs are the same.  The sum will have the same sign. */
      else
      {
        /* If the result is negative, do 10s complement now. */
        if(op1->sign == true)
        {
          if((retcode = bcd_tens_complement(sig1, sig1)) == false) break;
          overflow ^= 1;
        }

        /* If overflow, shift significand, insert overflow, bump exponent. */
        if(overflow != 0)
        {
          if((retcode = bcd_shift_significand(sig1, 1)) != true) break;
          if((retcode = bcd_sig_set_digit(sig1, 0, overflow)) != true) break;
          op1->exponent++;
        }
      }
      BCD_PRINT(BCD_DBG_OP_ADD, "%s()        OVERFLOW: %s %d %d.\n", __func__, bcd_sig_to_str(sig1), op1->exponent, op1->sign);

      /* Clear out any leading zeroes in the significand. */
      if((retcode = bcd_sig_remove_leading_zeroes(sig1, &op1->exponent)) != true) break;
      BCD_PRINT(BCD_DBG_OP_ADD, "%s()    CLEAR ZEROES: %s %d %d.\n", __func__, bcd_sig_to_str(sig1), op1->exponent, op1->sign);

      /* Done.  Set the object to reflect the fact that we calculated the value.
       * This is no longer data that came in through bcd_add_char(). */
      op1->char_count        = 0;
      op1->got_decimal_point = false;
      retcode                = true;
    } while(0);
  }
    
  return retcode;
}

/* This is the BCD subtraction function.  Subtraction is nothing more than 10's
 * complement addition.
 *
 * Input:
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Subtraction is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the difference.
 *   false = failure.
 */
bool
bcd_op_sub(bcd *op1,
           bcd *op2)
{
  bool retcode = false;

  if((op1 != (bcd *) 0) && (op2 != (bcd *) 0))
  {
    /* op1 - 0 = op1. */
    if(bcd_sig_is_zero(&op2->significand) == true)
    {
      retcode = true;
    }

    /* 0 - op2 = -op2. */
    else if(bcd_sig_is_zero(&op1->significand) == true)
    {
      if(bcd_copy(op2, op1) == true)
      {
        op1->sign = (op1->sign == true) ? false : true;
        retcode = true;
      }
    }

    /* op1 and op2 are both != 0. */
    else do
    {
      /* Start with the raw significands. */
      significand_t *sig1 = &op1->significand;
      significand_t *sig2 = &op2->significand;
      BCD_PRINT(BCD_DBG_OP_SUB, "%s()           BEGIN: %s - %s\n", __func__, bcd_sig_to_str(sig1), bcd_sig_to_str(sig2));

      /* If the exponents aren't the same, adjust the smaller number up to the other. */
      if((retcode = bcd_make_exponents_equal(sig1, &op1->exponent, sig2, &op2->exponent)) != true) break;

      /* 10s complement (as required):
       * POS - POS  = 10's complement b.
       * POS - NEG  = NO 10's complement.
       * NEG - POS  = NO 10's complement.
       * NEG - NEG  = 10's complement a.
       */
      if((op1->sign == false) && (op2->sign == false)) { if((retcode = bcd_tens_complement(sig2, sig2)) == false) break; }
      if((op1->sign ==  true) && (op2->sign ==  true)) { if((retcode = bcd_tens_complement(sig1, sig1)) == false) break; }
      BCD_PRINT(BCD_DBG_OP_SUB, "%s() 10'S COMPLEMENT: %s, %s\n", __func__, bcd_sig_to_str(sig1), bcd_sig_to_str(sig2));

      uint8_t overflow;
      if((retcode = bcd_significand_add(sig1, sig2, sig1, NULL, &overflow)) != true) break;
      BCD_PRINT(BCD_DBG_OP_SUB, "%s():         RESULT: %s CARRY: %d.\n", __func__, bcd_sig_to_str(sig1), overflow);

      /* 10s complement (as required) and set the result sign:
       * POS - POS  = Sign is defined by overflow.
       * POS - NEG  = Sign is positive.
       * NEG - POS  = Sign is negative.
       * NEG - NEG  = Sign is defined by overflow.
       */
      if((op1->sign == false) && (op2->sign == false))
      {
        op1->sign = ((overflow != 0) ? false : true);
        if(op1->sign == true)
        {
          if((retcode = bcd_tens_complement(sig1, &op1->significand)) != true) break;
          BCD_PRINT(BCD_DBG_OP_SUB, "%s():       NEGATIVE: %s.\n", __func__, bcd_sig_to_str(sig1));
        }
      }
      else if((op1->sign == true) && (op2->sign == true))
      {
        op1->sign = (overflow != 0) ? false : true;
        if(op1->sign == true)
        {
          if((retcode = bcd_tens_complement(sig1, &op1->significand)) != true) break;
          BCD_PRINT(BCD_DBG_OP_SUB, "%s():       NEGATIVE: %s.\n", __func__, bcd_sig_to_str(sig1));
        }
      }
      else
      {
        op1->sign = (op1->sign == false) ? false : true;
      }

      /* Clear out any leading zeroes in the significand. */
      if((retcode = bcd_sig_remove_leading_zeroes(sig1, &op1->exponent)) != true) break;
      BCD_PRINT(BCD_DBG_OP_SUB, "%s():          SHIFT: %s %d %d.\n", __func__, bcd_sig_to_str(sig1), op1->exponent, op1->sign);

      /* Done.  Set the object to reflect the fact that we calculated the value.
       * This is no longer data that came in through bcd_add_char(). */
      op1->char_count        = 0;
      op1->got_decimal_point = false;
    } while(0);
  }
    
  return retcode;
}

/* This is the BCD multiplication function.
 *
 * Input:
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Multiplication is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the product.
 *   false = failure.
 */
bool
bcd_op_mul(bcd *op1,
           bcd *op2)
{
  bool retcode = false;

  if((op1 != (bcd *) 0) && (op2 != (bcd *) 0))
  {
    do
    {
      /* Start with the raw significands. */
      significand_t *sig1 = &op1->significand;
      significand_t *sig2 = &op2->significand;
      BCD_PRINT(BCD_DBG_OP_MUL, "%s():          BEGIN: %s * %s\n", __func__, bcd_sig_to_str(sig1), bcd_sig_to_str(sig2));

      /* Normalize both numbers before we begin.  We need the exponents. */
      if((retcode = bcd_sig_remove_leading_zeroes(sig1, &op1->exponent)) != true) break;
      if((retcode = bcd_sig_remove_leading_zeroes(sig2, &op2->exponent)) != true) break;

      /* We'll store the result here. */
      significand_t result_hi = { .s = { 0 } }, result_lo = { .s = { 0 } };

      bool carry = false;

      int b_digit;
      for(b_digit = BCD_NUM_DIGITS; b_digit > 0; b_digit--)
      {
        int a_digit;
        for(a_digit = BCD_NUM_DIGITS; a_digit > 0; a_digit--)
        {
          /* 1. Get the 2 digits into a_byte and b_byte.
           *
           * 2. If either is zero, continue (zero time anything is zero).
           *
           * 3. Multiply them.  This creates a hex result (stored in prod).
           *
           * 4. Convert the hex result into a base-10 result (res_digit) and
           *    remainder (rem_digit).
           */
          uint8_t a_byte = bcd_sig_get_digit(sig1, (a_digit - 1));
          uint8_t b_byte = bcd_sig_get_digit(sig2, (b_digit - 1));
          if((a_byte == 0) || (b_byte == 0)) continue;
          uint8_t prod = a_byte * b_byte;
          uint8_t res_digit = (prod / 10);
          uint8_t rem_digit = (prod % 10);

          carry = (res_digit > 0) ? true : false;

          /* 1. Set remain_digit and result_digit to the position (relative to
           *    the left-hand edge of result_hi:result_lo) where the result and
           *    remainder should be added into the total.
           * 
           * 2. Set remain_sig and result_sig to point to either result_hi or
           *    result_lo, depending on where they fall in the result space.
           *
           * 3. Adjust remain_digit and result_digit to their position (relative
           *    to either result_hi or result_lo).
           *
           * 4. Set result and remain to contain the actual result and remain
           *    digits (positioned correctly).
           */
          int remain_digit = (b_digit + a_digit) - 1;
          int result_digit = remain_digit - 1;
          significand_t *remain_sig = (remain_digit < BCD_NUM_DIGITS_INTERNAL) ? &result_hi : &result_lo;
          significand_t *result_sig = (result_digit < BCD_NUM_DIGITS_INTERNAL) ? &result_hi : &result_lo;
          remain_digit %= BCD_NUM_DIGITS_INTERNAL;
          result_digit %= BCD_NUM_DIGITS_INTERNAL;
          significand_t remain;
          significand_t result;
          if((retcode = bcd_sig_initialize(&remain))                         != true) break;
          if((retcode = bcd_sig_initialize(&result))                         != true) break;
          if((retcode = bcd_sig_set_digit(&remain, remain_digit, rem_digit)) != true) break;
          if((retcode = bcd_sig_set_digit(&result, result_digit, res_digit)) != true) break;

          /* Create this, in case we need to do an overflow addition. */
          significand_t overflo_val = { .s = { 0 } };
          overflo_val.s[SIGNIFICAND_SECTIONS_INTERNAL - 1] = 1;

          /* Now we're ready to add remain and result to the result_hi:result_lo total. */
          uint8_t overflow;
          bool tmp_carry = false;
          if(rem_digit != 0)
          {
            if((retcode = bcd_significand_add(remain_sig, &remain, remain_sig, &tmp_carry, &overflow)) != true) break;
            carry = (tmp_carry == true) ? true : carry;

            if((overflow != 0) && (remain_sig == &result_lo))
            {
              if((retcode = bcd_significand_add(&result_hi, &overflo_val, &result_hi, &tmp_carry, &overflow)) != true) break;
              carry = (tmp_carry == true) ? true : carry;
            }
          }
          if(res_digit != 0)
          {
            if((retcode = bcd_significand_add(result_sig, &result, result_sig, &tmp_carry, &overflow)) != true) break;
            carry = (tmp_carry == true) ? true : carry;

            if((overflow != 0) && (result_sig == &result_lo))
            {
              if((res_digit != 0) && (retcode = bcd_significand_add(&result_hi, &overflo_val, &result_hi, NULL, &overflow)) != true) break;
            }
          }

          BCD_PRINT(BCD_DBG_OP_MUL, "%s(): RES: %s:%s %s %d\n", __func__, bcd_sig_to_str(&result_hi), bcd_sig_to_str(&result_lo), carry ? "true" : "false", overflow);
        }
      }
      if(retcode != true) break;

      /* Shift the result. */
      op1->significand = result_lo;
      while(bcd_sig_is_zero(&result_hi) == false)
      {
        if((retcode = bcd_shift_significand(&op1->significand, 1))  != true) { break; }
        uint8_t c = bcd_sig_get_digit(&result_hi, (BCD_NUM_DIGITS_INTERNAL - 1));
        if((retcode = bcd_shift_significand(&result_hi, 1))         != true) { break; }
        if((retcode = bcd_sig_set_digit(&op1->significand, 0, c))   != true) { break; }
      }

      /* If the result is zero, then set the exponent to zero and leave. */
      if(bcd_sig_is_zero(&op1->significand) == true)
      {
        op1->exponent = 0;
      }

      else
      {
        /* Set the exponent. */
        op1->exponent += op2->exponent;
        if(carry == true) op1->exponent++;
      }

      /* Set the sign. */
      op1->sign = (op1->sign == op2->sign) ? false : true;

      BCD_PRINT(BCD_DBG_OP_MUL, "%s(): OUT: %s %s %d\n", __func__, bcd_sig_to_str(&op1->significand), op1->sign ? "neg" : "pos", op1->exponent);

      /* Done.  Set the object to reflect the fact that we calculated the value.
       * This is no longer data that came in through bcd_add_char(). */
      op1->char_count        = 0;
      op1->got_decimal_point = false;
    } while(0);
  }
    
  return retcode;
}

/* This is the BCD division function.
 *
 * Input:
 *   op1  = A pointer to the dividend.  The result is returned in this one.
 *
 *   op2  = A pointer to the divisor.
 *
 * Output:
 *   true  = success.  op1 contains the quotient.
 *   false = failure.
 */
bool
bcd_op_div(bcd *op1,
           bcd *op2)
{
  bool retcode = false;

  if((op1 != (bcd *) 0) && (op2 != (bcd *) 0))
  {
    do
    {
      BCD_PRINT(BCD_DBG_OP_DIV, "%s() BEGIN: %s / %s\n", __func__, bcd_sig_to_str(&op1->significand), bcd_sig_to_str(&op2->significand));

      /* Check for divide by zero. */
      if(bcd_sig_is_zero(&op2->significand) == true) break;

      /* Delete leading zeroes from the divisor.  Adjust the exponent too.  We
       * only need to do this if we're dealing with a user-supplied number.
       * All other numbers won't have leading zeroes. */
      if((retcode = bcd_sig_remove_leading_zeroes(&op2->significand, &op2->exponent)) != true) break;

      /* We need to work in large data so we have room to calculate a fullsize
       * quotient.  This allows for rounding at the end of the quotient. */
      significand_t dividend_hi, dividend_lo;
      significand_t  divisor_hi,  divisor_lo;
      significand_t   result_hi,   result_lo;
      significand_t     mask_hi,     mask_lo;
      significand_t  add_one_hi,  add_one_lo;
      if(bcd_sig_initialize(&dividend_hi) != true) break;
      if(bcd_sig_initialize(&dividend_lo) != true) break;
      if(bcd_sig_initialize( &divisor_hi) != true) break;
      if(bcd_sig_initialize( &divisor_lo) != true) break;
      if(bcd_sig_initialize(  &result_hi) != true) break;
      if(bcd_sig_initialize(  &result_lo) != true) break;
      if(bcd_sig_initialize(    &mask_hi) != true) break;
      if(bcd_sig_initialize(    &mask_lo) != true) break;
      if(bcd_sig_initialize(& add_one_hi) != true)  break;
      if(bcd_sig_initialize(& add_one_lo) != true)  break;

      if(bcd_sig_copy(&op1->significand, &dividend_hi) != true) break;
      if(bcd_sig_copy(&op2->significand,  &divisor_hi) != true) break;

      /* Set mask_hi to mark the range of significant digits in the divisor. */
      {
        int i;
        if(bcd_sig_set_digit(&mask_hi, 0, 0xF) != true) break;
        for(i = 0; (i < BCD_NUM_DIGITS) && (bcd_sig_cmp(&divisor_hi, &mask_hi, &divisor_hi, 0) != 0); i++)
        {
          if(bcd_sig_set_digit(&mask_hi, i, 0xF) != true) break;
        }
        BCD_PRINT(BCD_DBG_OP_DIV, "%s() MASK: Divisor %s: Mask %s\n", __func__, bcd_sig_to_str(&divisor_hi), bcd_sig_to_str(&mask_hi));
      }

      /* This identifies the digit position where we're currently calculating
       * the quotient.  Each time we subtract the divisor from the dividend we
       * will add 1 to the quotient. */
      if(bcd_sig_set_digit(&add_one_hi, 0, 1) != true) break;

      /* Loop here until we're done with the division.  We go until we have as
       * much of 2 full significands as possible.  This should give us at least
       * one extra digit (to allow for rounding).  Then we're done. */
      bool done = false;
      while(done == false)
      {
        /* Loop here as long as dividend (with mask) is >= divisor (with mask).
         * There are 2 different tests that we do in order to figure that out:
         * 1. dividend_hi > divisor_hi.
         * 2. (dividend_hi == divisor_hi) && (dividend_lo >= divisor_lo). */
        while( (bcd_sig_cmp(&divisor_hi, &mask_hi, &dividend_hi, &mask_hi) < 0) ||
              ((bcd_sig_cmp(&divisor_hi, &mask_hi, &dividend_hi, &mask_hi) == 0) && (bcd_sig_cmp(&divisor_lo, &mask_lo, &dividend_lo, &mask_lo) <= 0)) )
        {
          BCD_PRINT(BCD_DBG_OP_DIV, "%s() LOOP_TOP: DIVIDEND: %s:%s\n", __func__, bcd_sig_to_str(&dividend_hi), bcd_sig_to_str(&dividend_lo));
          BCD_PRINT(BCD_DBG_OP_DIV, "%s() LOOP_TOP: DIVISOR:  %s:%s\n", __func__, bcd_sig_to_str(&divisor_hi),  bcd_sig_to_str(&divisor_lo));
          BCD_PRINT(BCD_DBG_OP_DIV, "%s() LOOP_TOP: MASK:     %s:%s\n", __func__, bcd_sig_to_str(&mask_hi),     bcd_sig_to_str(&mask_lo));
          BCD_PRINT(BCD_DBG_OP_DIV, "%s() LOOP_TOP: RESULT:   %s:%s\n", __func__, bcd_sig_to_str(&result_hi),   bcd_sig_to_str(&result_lo));

          uint8_t overflow;
          significand_t value_tens;

          significand_t *res = (bcd_sig_is_zero(&divisor_hi) == false) ? &result_hi  : &result_lo;
          significand_t *one = (bcd_sig_is_zero(&divisor_hi) == false) ? &add_one_hi : &add_one_lo;
          if(bcd_significand_add(res, one, res, NULL, &overflow) == false) break;

          bool borrow = (bcd_sig_cmp(&dividend_lo, 0, &divisor_lo, 0) < 0) ? true : false;
          if(bcd_tens_complement(&divisor_lo, &value_tens)                                   == false) break;
          if(bcd_significand_add(&dividend_lo, &value_tens, &dividend_lo, NULL, &overflow)   == false) break;

          if(borrow == true)
          {
            significand_t one;
            if(bcd_sig_initialize(&one) != true)  break;
            if(bcd_sig_set_digit(&one, (BCD_NUM_DIGITS - 1), 0x1) != true) break;
            if(bcd_tens_complement(&one, &value_tens)                                        == false) break;
            if(bcd_significand_add(&dividend_hi, &value_tens, &dividend_hi, NULL, &overflow) == false) break;
          }

          if(bcd_tens_complement(&divisor_hi, &value_tens)                                   == false) break;
          if(bcd_significand_add(&dividend_hi, &value_tens, &dividend_hi, NULL, &overflow)   == false) break;

          BCD_PRINT(BCD_DBG_OP_DIV, "%s() LOOP_BOT: DIVIDEND: %s:%s\n", __func__, bcd_sig_to_str(&dividend_hi), bcd_sig_to_str(&dividend_lo));
          BCD_PRINT(BCD_DBG_OP_DIV, "%s() LOOP_BOT: DIVISOR:  %s:%s\n", __func__, bcd_sig_to_str(&divisor_hi),  bcd_sig_to_str(&divisor_lo));
          BCD_PRINT(BCD_DBG_OP_DIV, "%s() LOOP_BOT: MASK:     %s:%s\n", __func__, bcd_sig_to_str(&mask_hi),     bcd_sig_to_str(&mask_lo));
          BCD_PRINT(BCD_DBG_OP_DIV, "%s() LOOP_BOT: RESULT:   %s:%s\n", __func__, bcd_sig_to_str(&result_hi),   bcd_sig_to_str(&result_lo));
        }

        uint8_t c;
        if((c = bcd_sig_get_digit(&divisor_hi, (BCD_NUM_DIGITS - 1))) == 0xF) break;
        if(bcd_shift_significand(&divisor_hi,       1) == false) break;
        if(bcd_shift_significand(&divisor_lo,       1) == false) break;
        if(bcd_sig_set_digit(&divisor_lo, 0, c) == false) break;

        if((c = bcd_sig_get_digit(&add_one_hi, (BCD_NUM_DIGITS - 1))) == 0xF) break;
        if(bcd_shift_significand(&add_one_hi, 1) == false) break;
        if(bcd_shift_significand(&add_one_lo, 1) == false) break;
        if(bcd_sig_set_digit(&add_one_lo, 0, c) == false) break;

        c = bcd_sig_get_digit(&mask_hi, (BCD_NUM_DIGITS - 1));
        if(bcd_shift_significand(&mask_hi, 1) == false) break;
        if(bcd_sig_set_digit(&mask_hi, 0, 0xF) == false) break;
        if(bcd_sig_get_digit(&mask_lo, (BCD_NUM_DIGITS - 1)) == 0xF) { done = true; }
        if(bcd_shift_significand(&mask_lo, 1) == false) break;
        if(bcd_sig_set_digit(&mask_lo, 0, c) == false) break;
      }
      BCD_PRINT(BCD_DBG_OP_DIV, "%s() RESULT: %s:%s\n", __func__, bcd_sig_to_str(&result_hi), bcd_sig_to_str(&result_lo));

      /* Copy the result to op1 so we can return it to the caller. */
      if(bcd_sig_copy(&result_hi, &op1->significand) == false) break;

      /* Set the exponent, and then adjust to account for any leading zeroes. */
      {
        bool shift_ok = true;
        op1->exponent -= op2->exponent;
        while((bcd_sig_is_zero(&op1->significand) == false) && ((bcd_sig_get_digit(&op1->significand, 0)) == 0))
        {
          char c;
          if(bcd_shift_significand(&op1->significand, -1) != true)                   { shift_ok = false; break; }
          if((c = bcd_sig_get_digit(&result_lo, 0)) == 0xF)                           { shift_ok = false; break; }
          if(bcd_sig_set_digit(&op1->significand, (BCD_NUM_DIGITS - 1), c) == false) { shift_ok = false; break; }
          if(bcd_shift_significand(&result_lo, -1) != true)                          { shift_ok = false; break; }
          op1->exponent--;
        }
        if(shift_ok == false) break;
      }

      /* Set the sign. */
      op1->sign = (op1->sign == op2->sign) ? false : true;

      /* Now we need to round the result (if necessary).  We'll use the regular
       * bcd_op_add() function to do that step. */
      char c;
      if((c = bcd_sig_get_digit(&result_lo, 0)) == 0xF) break;
      if(c > 4)
      {
        bcd *round;
        if((round = bcd_new()) == (bcd *) NULL) break;
        if(bcd_sig_set_digit(&round->significand, (BCD_NUM_DIGITS - 1), 1) == false) break;
        round->sign = op1->sign;
        round->exponent = op1->exponent;
        if(bcd_op_add(op1, round) == false) break;
      }

      /* Done.  Set the object to reflect the fact that we calculated the value.
       * This is no longer data that came in through bcd_add_char(). */
      op1->char_count        = 0;
      op1->got_decimal_point = false;
      retcode                = true;
    } while(0);
  }
    
  return retcode;
}

/* This is the BCD exponentiation function.
 *
 * Input:
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Exponentiation is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the result.
 *   false = failure.
 */
bool
bcd_op_exp(bcd *op1,
           bcd *op2)
{
  bool retcode = false;

  do
  {
    fp_exp *fp;

    if((op1 == (bcd *) 0) || (op2 == (bcd *) 0))    { break; }

    if((fp = fp_exp_new(op1, op2)) == (fp_exp *) 0) { break; }

    if(fp_exp_calc(fp) == false)                    { break; }

    if(fp_exp_get_result(fp, op1) == false)         { break; }

    retcode = fp_exp_delete(fp);
  } while(0);
    
  return retcode;
}

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Create a new bcd object.  This object can be used to access the bcd class.
 *
 * Input:
 *   N/A.
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.
 */
bcd *
bcd_new(void)
{
  bcd *this = malloc(sizeof(*this));

  if(this != (bcd *) 0)
  {
    if(bcd_import(this, 0) == false)
    {
      bcd_delete(this);
      this = (bcd *) 0;
    }
  }

  return this;
}

/* Delete a bcd object that was created by bcd_new().
 *
 * Input:
 *   this = A pointer to the bcd object.
 *
 * Output:
 *   true  = success.  this is deleted.
 *   false = failure.  this is undefined.
 */
bool
bcd_delete(bcd *this)
{
  bool retcode = false;

  if(this != (bcd *) 0)
  {
    free(this);
    retcode = true;
  }

  return retcode;
}

/* Check to see if the specified character is a valid operand character that
 * can be passed to bcd_add_char().
 *
 * Input:
 *   c = The character to check.
 *
 * Output:
 *   true  = Yes, c is a valid operand character.
 *   false = No, c is NOT a valid operand character.
 */
bool
bcd_add_char_is_valid_operand(char c)
{
  bool retcode = ((c == '.') || ((c & 0xDF) == 'S') || ((c >= '0') && (c <= '9'))) ? true : false;

  return retcode;
}

/* Attempt to add a character to the bcd object.  The character is checked to
 * see if it's a valid part of a decimal number.  If it's valid, it is added.
 * Note that it's possible for a character to be dropped because we don't have
 * room for it.  Or if we get multiple decimal points we'll drop all subsequent
 * ones.
 *
 * We only allow the user to insert 16 digits.  After that, we drop them.
 *
 * Input:
 *   this = A pointer to the bcd object.
 *
 *   c    = The char to add.  If it's valid, we use it.  If it's not valid,
 *          then we return false.
 *
 * Output:
 *   true  = success.  c is valid, and it has been added to this.
 *   false = failure.  c is NOT a number OR we were unable to add c to this.
 */
bool
bcd_add_char(bcd *this,
             char c)
{
  bool retcode = false;

  if(this != (bcd *) 0)
  {
    /* If it's a decimal point, prepare to start doing decimal math.  If we
     * already got a decimal point, then this one is silently dropped. */
    if(c == '.')
    {
      /* If this is the first significant character, drop the exponent so we can
       * handle BCD_NUM_DIGITS digits. */
      if(this->char_count == 0)
      {
        this->exponent = -1;
      }

      this->got_decimal_point = true;
      retcode = true;
    }

    /* An 'S' toggles the +/- sign. */
    else if((c & 0xDF) == 'S')
    {
      this->sign ^= 1;
      retcode = true;
    }

    /* Not a decimal point or sign.  It better be a digit. */
    else if((c >= '0') && (c <= '9'))
    {
      /* Convert it to a binary. */
      c -= '0';

      /* If this is a leading (insignificant) zero, drop it. */
      if((c == 0) && (this->got_decimal_point == false) && (bcd_sig_is_zero(&this->significand) == true))
      {
      }

      /* If the significand is already full, then silently drop the character. */
      else if(this->char_count < BCD_NUM_DIGITS)
      {
        if((this->got_decimal_point == false) && (bcd_sig_is_zero(&this->significand) == false))
        {
          this->exponent++;
        }

        bcd_sig_set_digit(&this->significand, this->char_count++, c);
        BCD_PRINT(BCD_DBG_ADD_CHAR, "%s(): %s %d\n", __func__, bcd_sig_to_str(&this->significand), this->exponent);
      }

      retcode = true;
    }
  }

  return retcode;
}

/* Create an ASCII string the represents the current value of the number.
 *
 * Input:
 *   this     = A pointer to the bcd object.
 *
 *   buf      = The caller-supplied buffer to build the string in.
 *
 *   buf_size = The size of buf.  Note that we must allow 1 byte for the NULL
 *              terminator.
 *
 * Output:
 *   true  = success.  buf contains the string.  Note that it might be
 *                     truncated if buf is too small to hold the entire size of
 *                     the string.
 *   false = failure.  buf is undefined.
 */
bool
bcd_to_str(bcd  *this,
           char  *buf,
           size_t buf_size)
{
  bool retcode = false;

  if( (this != (bcd *) 0) && (buf != (char *) 0) && (buf_size > 0) )
  {
    BCD_PRINT(BCD_DBG_TO_STR, "%s(): %s, %d, %d, %d.\n", __func__,
              bcd_sig_to_str(&this->significand), this->exponent, this->got_decimal_point, this->sign);

    /* We need a buffer that's big enough to hold the number, and the number
     * has to fit within (sizeof(significand_t)) digits. */
    int16_t max_exp = (BCD_NUM_DIGITS - 1);
    int16_t min_exp = (0 - max_exp);
    int16_t exp = this->exponent;
    if((exp <= max_exp) && (exp >= min_exp))
    {
      /* Regular notation (1,222,333). */
      retcode = bcd_to_str_decimal(&this->significand,
                                    this->exponent,
                                    this->char_count,
                                    this->got_decimal_point,
                                    this->sign,
                                    buf,
                                    buf_size);
    }
    else
    {
      /* Need to use scientific notation (1.234e18). */
      retcode = bcd_to_str_decimal(&this->significand,
                                    0,
                                    0,
                                    false,
                                    this->sign,
                                    buf,
                                    buf_size);

      /* Now add the exponent. */
      do
      {
        /* Set buf_x to the end of the existing string, and then adjust buf_size
         * to account for the data that was already placed in the buffer. */
        int buf_x = strlen(buf);
        buf_size -= buf_x;

        /* Add the 'e' to designate exponent.  Then add the exponent sign. */
        if(--buf_size == 0) { break; } else { buf[buf_x++] = 'e'; }
        if(--buf_size == 0) { break; } else { buf[buf_x++] = (this->exponent < 0) ? '-' : '+'; }

        int16_t exponent = (this->exponent < 0) ? (0 - this->exponent) : this->exponent;
        int num;
        for(num = 10000; (num >= 1) && ((exponent / num) == 0); num /= 10);
        for(; num >= 1; num /= 10)
        {
          int16_t digit = (exponent / num);
          char c = digit + '0';
          if(--buf_size == 0) { break; } else { buf[buf_x++] = c; }
          exponent -= (digit * num);
        }
        buf[buf_x] = 0;
      } while(0);
    }
  }

  return retcode;
}

/* Make a copy of a BCD object.  The caller provides a src and dst, and this
 * member makes a copy.
 *
 * Input:
 *   src = A pointer to the bcd object.
 *
 *   dst = A pointer to a pre-allocated bcd object that we will copy into.
 *
 * Output:
 *   true  = success.  src has been copied to dst.
 *   false = failure.  The contents of dst is undefined.
 */
bool
bcd_copy(bcd *src,
         bcd *dst)
{
  bool retcode = false;

  if( (src != (bcd *) 0) && (dst != (bcd *) 0) )
  {
    if((retcode = bcd_sig_copy(&src->significand, &dst->significand)) == true)
    {
      dst->exponent          = src->exponent;
      dst->sign              = src->sign;
      dst->char_count        = src->char_count;
      dst->got_decimal_point = src->got_decimal_point;
    }
  }

  return retcode;
}

/* Compare 2 bcd objects.
 *
 * Input:
 *   obj1 = A pointer to one of the bcd objects.
 *
 *   obj2 = A pointer to one of the bcd objects.
 *
 * Output:
 *   -1 if *obj1 <  *obj2.
 *    0 if *obj1 == *obj2.
 *    1 if *obj1 >  *obj2.
 */
int
bcd_cmp(bcd *obj1,
        bcd *obj2)
{
  int retval = 0;

  if( (obj1 != (bcd *) 0) && (obj2 != (bcd *) 0) )
  {
    /* Negatives are always less than positives. */
    if((obj1->sign == true) && (obj2->sign == false))
    {
      retval = -1;
    }
 
    /* Positives are always greater than negatives. */
    else if((obj1->sign == false) && (obj2->sign == true))
    {
      retval =  1;
    }

    /* If they're either both positive or both negative. */
    else
    {
      bcd *tmp_obj1 = bcd_new();
      bcd *tmp_obj2 = bcd_new();
      if((tmp_obj1 != (bcd *) 0) &&
         (tmp_obj2 != (bcd *) 0) &&
         (bcd_copy(obj1, tmp_obj1) == true) &&
         (bcd_copy(obj2, tmp_obj2) == true))
      {
        /* Compare 2 positive numbers. */
        if((obj1->sign == false) && (obj2->sign == false))
        {
          if(bcd_op_sub(tmp_obj1, tmp_obj2) == true)
          {
            if(bcd_sig_is_zero(&tmp_obj1->significand)) { retval =  0; }
            else if(tmp_obj1->sign == true)             { retval = -1; }
            else                                        { retval =  1; }
          }
        }


        /* Compare 2 negative numbers. */
        else
        {
          if(bcd_op_sub(tmp_obj1, tmp_obj2) == true)
          {
            if(bcd_sig_is_zero(&tmp_obj1->significand)) { retval =  0; }
            else if(tmp_obj1->sign == false)            { retval =  1; }
            else                                        { retval = -1; }
          }
        }
      }

      bcd_delete(tmp_obj2);
      bcd_delete(tmp_obj1);
    }
  }

  return retval;
}

/* Import a signed integer value into this object.  Note that this member
 * doesn't allow you to import an IEEE floating point value.
 *
 * Input:
 *   this     = A pointer to the bcd object.
 *
 *   src      = A signed integer value to use to seed the object.  If there is
 *              a value already loaded into this object, it will be erased.
 *
 * Output:
 *   true  = success.  this has been imported.
 *   false = failure.  The contents of this is undefined.
 */
bool
bcd_import(bcd     *this,
           int64_t  src)
{
  bool retcode = false;

  do
  {
    if(this == (bcd *) 0)                                          { break; }

    if(bcd_sig_initialize(&this->significand) == false)            { break; }

    /* Set the sign, and then set src = |src|. */
    this->sign = 0;
    if(src < 0)
    {
      this->sign = 1;
      src = (0 - src);
    }

    this->exponent = 0;
    while(src != 0)
    {
      uint8_t digit = (src % 10);

      if(bcd_shift_significand(&this->significand, 1) == false)    { break; }

      if(bcd_sig_set_digit(&this->significand, 0, digit) == false) { break; }

      if((src /= 10) != 0)
      {
        this->exponent++;
      }
    }

    this->got_decimal_point = false;
    this->char_count        = 0;

    retcode = true;
  } while(0);

  return retcode;
}

/* Export the value of this object to a signed integer.  Note that this member
 * doesn't allow you to export to an IEEE floating point value.
 *
 * Input:
 *   this     = A pointer to the bcd object.
 *
 *   dst      = A pointer to a signed integer value that will receive the value
 *              of this.
 *
 * Output:
 *   true  = success.  this has been exported.
 *   false = failure.  The contents of dst is undefined.
 */
bool
bcd_export(bcd     *this,
           int64_t *dst)
{
  bool retcode = false;

  do
  {
    if((this == (bcd *) 0) || (dst == (int64_t *) 0))             { break; }

    *dst = 0;

    retcode = true;

    if((this->exponent < 0) || (this->exponent > BCD_NUM_DIGITS)) { break; }

    int offset;
    for(offset = 0; offset <= this->exponent; offset++)
    {
      uint8_t digit = bcd_sig_get_digit(&this->significand, offset);

      *dst *= 10;
      *dst += digit;
    }

    /* Set the sign. */
    if(this->sign == 1)
    {
      *dst = (0 - *dst);
    }
  } while(0);

  return retcode;
}

/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#ifdef TEST
#define TEST_PRIMITIVES
#define TEST_CHAR_INPUT
#define TEST_MATH_OPERATIONS
#define TEST_SPECIAL

bool
bcd_test(void)
{
  bool retcode = false;

  /***********************************************
   * Test the primitives.
   **********************************************/
#ifdef TEST_PRIMITIVES
  {
    {
      printf("  Sig-Sect Tests (SIGNIFICAND_DIGITS_PER_SECTION = %d).\n", SIGNIFICAND_DIGITS_PER_SECTION);
      int j;
      significand_section_t test_sect = { 0xFFFFFFFF };
      for(j = 0; j < SIGNIFICAND_DIGITS_PER_SECTION; j++)
      {
        if(bcd_sect_set_digit(&test_sect, j, (j + 1)) != true)                                return false;
      }
      if(test_sect != 0x12345678)                                                             return false;
      for(j = 0; j < SIGNIFICAND_DIGITS_PER_SECTION; j++)
      {
        if(bcd_sect_get_digit(test_sect, j) != (j + 1))                                       return false;
      }
      int arg2 = (SIGNIFICAND_DIGITS_PER_SECTION + 1);
      if(bcd_sect_set_digit(&test_sect, arg2, 1) != false)                                    return false;
      if(bcd_sect_get_digit(test_sect, arg2) != 0xF)                                          return false;
    }

    {
      printf("  Sig Tests.\n");
      int j;
      significand_t test_sig = { .s = { 0xFFFFFFFF } };
      for(j = 0; j < BCD_NUM_DIGITS_INTERNAL; j++)
      {
        if(bcd_sig_set_digit(&test_sig, j, ((j + 1) & 0xF)) != true)                          return false;
      }
      significand_section_t sig_test_data[2] = { 0x12345678, 0x9ABCDEF0 };
      for(j = 0; j < SIGNIFICAND_SECTIONS_INTERNAL; j++)
      {
        if(test_sig.s[j] != sig_test_data[(j & 0x1)])                                         return false;
      }
      for(j = 0; j < BCD_NUM_DIGITS_INTERNAL; j++)
      {
        if(bcd_sig_get_digit(&test_sig, j) != ((j + 1) & 0xF))                                return false;
      }
      int arg2 = BCD_NUM_DIGITS_INTERNAL;
      if(bcd_sig_set_digit(&test_sig, arg2, 1) != false)                                      return false;
      if(bcd_sig_get_digit(&test_sig, arg2) != 0xF)                                           return false;
    }

    {
      printf("  Sig Manipulation Tests.\n");
      int j;
      significand_section_t sig_test_data1[2] = { 0x12345678, 0xFEDCBA98 };
      significand_section_t sig_test_data2[4] = { 0x00012345, 0x678FEDCB, 0xA9812345, 0x678FEDCB };
      significand_section_t sig_test_data3[4] = { 0x45678FED, 0xCBA98123, 0x45678FED, 0xCB000000 };
      significand_t sig1 = { .s = { 0 } };
      for(j = 0; j < SIGNIFICAND_SECTIONS_INTERNAL; j++)
      {
        sig1.s[j] = sig_test_data1[(j & 1)];
      }
      if(bcd_shift_significand(&sig1, 3) != true)                                             return false;
      for(j = 0; j < SIGNIFICAND_SECTIONS_INTERNAL; j++)
      {
        if(sig1.s[j] != sig_test_data2[j])                                                    return false;
      }
      if(bcd_shift_significand(&sig1, -6) != true)                                            return false;
      for(j = 0; j < SIGNIFICAND_SECTIONS_INTERNAL; j++)
      {
        if(sig1.s[j] != sig_test_data3[j])                                                    return false;
      }
      if(bcd_sig_is_zero(&sig1) != false)                                                     return false;
      if(bcd_sig_initialize(&sig1) != true)                                                   return false;
      if(bcd_sig_is_zero(&sig1) != true)                                                      return false;
    }

    {
      printf("  Sig Copy and Compare.\n");
      significand_t sig1 = { .s = { 0x12345678, 0xFEDCBA98 } };
      significand_t sig2 = { .s = { 0xFEDABC78, 0xFE501234 } };
      significand_t mask = { .s = { 0x000000FF, 0xFF000000 } };
      if(bcd_sig_cmp(&sig1,     0, &sig2,     0) != -1)                                       return false;
      if(bcd_sig_cmp(&sig1, &mask, &sig2, &mask) !=  0)                                       return false;
      if(bcd_sig_copy(&sig1, &sig2) != true)                                                  return false;
      if((sig1.s[0] != 0x12345678) || (sig1.s[1] != 0xFEDCBA98))                              return false;
      if(bcd_sig_cmp(&sig1, 0, &sig2, 0) != 0)                                                return false;
    }
  }
#endif // TEST_PRIMITIVES

#ifdef TEST_CHAR_INPUT
  /* Loop through some decimal numbers.  This tests the basic functionality of
   * the bcd class.  We're checking to make sure it can handle any type of
   * decimal number that we might throw at it. */
  typedef struct bcd_test {
    const char  *name;
    const char  *src;
    const char  *dst;
  } bcd_test;
  bcd_test tests[] = {
    { "BCD_01",                ""                 ,                     "0"                 }, // A blank object.
    { "BCD_02",               "1"                 ,                     "1"                 }, // A single-digit number (exp = 0).
    { "BCD_03",             "123"                 ,                   "123"                 }, // Simple integer value (exp > 0).
    { "BCD_04",             "123."                ,                   "123."                }, // User just entered a decimal point.
    { "BCD_05",          "123000"                 ,               "123,000"                 }, // Integer with trailing zeroes.
    { "BCD_06",       "000123000"                 ,               "123,000"                 }, // Insignificant leading zeroes.
    { "BCD_07",             "123.456"             ,                   "123.456"             }, // Simple floating point value.
    { "BCD_08",             "123.456000"          ,                   "123.456000"          }, // Insignificant trailing zeroes.
    { "BCD_09",             "123.456007"          ,                   "123.456007"          }, // Significant zeroes in middle of the decimal.
    { "BCD_10",             "000.000123"          ,                     "0.000123"          }, // Insignificant and significant zeroes.
    { "BCD_11",             "000.0123S"           ,                    "-0.0123"            }, // Negative number.
    { "BCD_12",                ".000000000000000" ,                     "0.000000000000000" }, // No significant digit.
    { "BCD_13",                ".000000000000001" ,                     "0.000000000000001" }, // One significant digit.
    { "BCD_14",                ".000123"          ,                     "0.000123"          },
    { "BCD_15","1222333444555666"                 , "1,222,333,444,555,666"                 }, // Commas in the right place.
  };
  size_t bcd_test_size = (sizeof(tests) / sizeof(bcd_test));

  {
    int x;
    for(x = 0; x < bcd_test_size; x++)
    {
      bcd_test *t = &tests[x];
      printf("  %s: '%s'.\n", t->name, t->src);

      bcd *this = bcd_new();
      if((retcode = (this != (bcd *) 0)) != true)                                             return false;

      const char *src = t->src;
      while(*src)
      {
        if((retcode = bcd_add_char(this, *src++)) != true)                                    return false;
      }

      char buf[1024];
      memset(buf, 0, sizeof(buf));
      if((retcode = bcd_to_str(this, buf, sizeof(buf))) != true)                              return false;

      DBG_PRINT("strcmp(%s, %s)\n", t->dst, buf);
      if((retcode = (strcmp(t->dst, buf) == 0)) != true) { printf("%s != %s\n", t->dst, buf); return false; }

      if((retcode = bcd_delete(this)) != true)                                                return false;
      this = (bcd *) 0;
    }
  }
#endif // TEST_CHAR_INPUT
  
#ifdef TEST_MATH_OPERATIONS
  /* Math operations. */
  typedef struct bcd_math_test {
    const char  *name;
    bool (*func)(bcd *val1, bcd *val2);
    const char  *val1;
    const char  *val2;
    const char  *result;
  } bcd_math_test;
  bcd_math_test math_tests[] = {
    { "BCD_ADD_01", bcd_op_add,                "1"                 ,                "2"                 ,                     "3"                     }, // Debug
    { "BCD_ADD_02", bcd_op_add,         "99999999"                 ,                "1"                 ,           "100,000,000"                     }, // 33-bits
    { "BCD_ADD_03", bcd_op_add,        "999999999"                 ,                "1"                 ,         "1,000,000,000"                     }, // Carry.
    { "BCD_ADD_04", bcd_op_add, "1234567890123456"                 , "9876543210987654"                 ,                     "1.111111110111111e+16" }, // 17 digits
    { "BCD_ADD_05", bcd_op_add,                 ".1234567890123456", "9876543210987654"                 , "9,876,543,210,987,654"                     }, // 16.0 + 0.16 digits.
    { "BCD_ADD_06", bcd_op_add,             "1234s"                ,             "4321"                 ,                 "3,087"                     }, // 1st num neg.
    { "BCD_ADD_07", bcd_op_add,             "8766"                 ,             "4321"                 ,                "13,087"                     }, // Like previous, but pos.
    { "BCD_ADD_08", bcd_op_add,              "123"                 ,             "1234"                 ,                 "1,357"                     }, // Pos + Pos.
    { "BCD_ADD_09", bcd_op_add,              "456s"                ,              "123"                 ,                  "-333"                     }, // Neg + Pos = Neg.
    { "BCD_ADD_10", bcd_op_add,              "456s"                ,             "1234"                 ,                   "778"                     }, // Neg + Pos = Pos.
    { "BCD_ADD_11", bcd_op_add,              "789"                 ,             "1234s"                ,                  "-445"                     }, // Pos + Neg = Neg.
    { "BCD_ADD_12", bcd_op_add,              "789"                 ,              "123s"                ,                   "666"                     }, // Pos + Neg = Pos.
    { "BCD_ADD_13", bcd_op_add,              "202s"                ,             "1234s"                ,                "-1,436"                     }, // Neg + Neg.
    { "BCD_ADD_14", bcd_op_add,             "9990s"                ,             "1234s"                ,               "-11,224"                     }, // Neg with carry.
    { "BCD_ADD_15", bcd_op_add,               "10.5"               ,                 ".5"               ,                    "11"                     }, // decimal to whole.
    { "BCD_ADD_16", bcd_op_add,                 ".1111111111111111",                 ".1111111111111111",                     "0.2222222222222222"    }, // No carry, no truncate.
    { "BCD_ADD_17", bcd_op_add,             "1000"                 ,             "1000"                 ,                 "2,000"                     }, // Trailing zeroes.
    { "BCD_ADD_18", bcd_op_add,                 ".00001"           ,                 ".00001"           ,                     "0.00002"               }, // Significant zeroes.
    { "BCD_ADD_19", bcd_op_add,                "1.0000134s"        ,                 ".045"             ,                    "-0.9550134"             },
    { "BCD_ADD_20", bcd_op_add, "9999999999999999"                 ,                "1"                 ,                     "1e+16"                 }, // Overflow
    { "BCD_ADD_21", bcd_op_add,         "99999999"                 ,                "1"                 ,           "100,000,000"                     }, // Carry up to next sect.
    { "BCD_ADD_22", bcd_op_add, "1111111111111111"                 ,                 ".9"               , "1,111,111,111,111,112"                     }, // Carry past the end.
    { "BCD_ADD_23", bcd_op_add, "9999999999999999"                 ,                 ".9"               ,                     "1e+16"                 }, // Carry all the way up.
    { "BCD_ADD_24", bcd_op_add, "6666666666666666"                 ,                 ".9"               , "6,666,666,666,666,670"                     }, // Carry a little bit.

    { "BCD_SUB_01", bcd_op_sub,                "5"                 ,                "2"                 ,                     "3"                     }, // Debug.
    { "BCD_SUB_02", bcd_op_sub,                "0"                 ,                "1"                 ,                    "-1"                     }, // Neg num.
    { "BCD_SUB_03", bcd_op_sub,            "12345"                 ,             "1234"                 ,                "11,111"                     }, // Pos - Pos = Pos.
    { "BCD_SUB_04", bcd_op_sub,            "54321"                 ,            "91234"                 ,               "-36,913"                     }, // Pos - Pos = Neg.
    { "BCD_SUB_05", bcd_op_sub,            "12345"                 ,              "123.4s"              ,                "12,468.4"                   }, // Pos - Neg = Pos.
    { "BCD_SUB_06", bcd_op_sub,              "432.1s"              ,                "7.5678"            ,                  "-439.6678"                }, // Neg - Pos = Neg.
    { "BCD_SUB_07", bcd_op_sub,             "1225s"                ,               "34.95s"             ,                "-1,190.05"                  }, // Neg - Neg = Neg.
    { "BCD_SUB_08", bcd_op_sub, "1111111111111111s"                , "1234567890123456s"                ,   "123,456,779,012,345"                     }, // Neg - Neg = Pos.
    { "BCD_SUB_09", bcd_op_sub,                "3"                 ,                "0"                 ,                     "3"                     }, // Val - 0 = Val.
    { "BCD_SUB_10", bcd_op_sub,                "0"                 ,           "452389.841"             ,              "-452,389.841"                 }, // 0 - +Val = -Val.
    { "BCD_SUB_11", bcd_op_sub,                "0"                 ,                 ".2841s"           ,                     "0.2841"                }, // 0 - -Val = +Val.
    { "BCD_SUB_11", bcd_op_sub,                "0"                 ,                "0"                 ,                     "0"                     }, // 0 - 0 = 0.

    { "BCD_MUL_01", bcd_op_mul,                "3"                 ,                "2"                 ,                     "6"                     }, // Debug.
    { "BCD_MUL_02", bcd_op_mul,             "4567"                 ,            "56789"                 ,           "259,355,363"                     }, // Lots of carry.
    { "BCD_MUL_03", bcd_op_mul,                "1"                 ,                "0"                 ,                     "0"                     }, // Non-0 * 0 = 0.
    { "BCD_MUL_04", bcd_op_mul,                "0"                 ,                "8"                 ,                     "0"                     }, // 0 * Non-0 = 0.
    { "BCD_MUL_05", bcd_op_mul,            "87878"                 ,             "4539.123"             ,       "398,889,050.994"                     }, // Pos * Pos = Pos.
    { "BCD_MUL_06", bcd_op_mul,            "13579.2468"            ,                 ".8579s"           ,               "-11,649.63582972"            }, // Pos * Neg = Neg.
    { "BCD_MUL_07", bcd_op_mul,                "1.0000134s"        ,                 ".045"             ,                    "-0.045000603"           }, // Neg * Pos = Neg.
    { "BCD_MUL_08", bcd_op_mul,       "5579421358s"                ,               "42s"                ,       "234,335,697,036"                     }, // Neg * Neg = Pos.
    { "BCD_MUL_09", bcd_op_mul,               "13.57900000"        ,             "8700.0000"            ,               "118,137.3"                   }, // Insignificant zeroes.
    { "BCD_MUL_10", bcd_op_mul, "9999999999999999"                 ,                "9"                 ,                     "8.999999999999999e+16" }, // Very large numbers. 
    { "BCD_MUL_11", bcd_op_mul, "9999999999999999"                 ,               "99"                 ,                     "9.899999999999999e+17" },
    { "BCD_MUL_12", bcd_op_mul, "9999999999999999"                 ,              "999"                 ,                     "9.989999999999999e+18" },
    { "BCD_MUL_13", bcd_op_mul, "9999999999999999"                 ,             "9999"                 ,                     "9.998999999999999e+19" },
    { "BCD_MUL_14", bcd_op_mul, "9999999999999999"                 ,            "99999"                 ,                     "9.999899999999999e+20" },
    { "BCD_MUL_15", bcd_op_mul, "9999999999999999"                 ,           "999999"                 ,                     "9.999989999999999e+21" },
    { "BCD_MUL_16", bcd_op_mul, "9999999999999999"                 ,          "9999999"                 ,                     "9.999998999999999e+22" },
    { "BCD_MUL_17", bcd_op_mul, "9999999999999999"                 ,         "99999999"                 ,                     "9.999999899999999e+23" },
    { "BCD_MUL_18", bcd_op_mul, "9999999999999999"                 ,        "999999999"                 ,                     "9.999999989999999e+24" },
    { "BCD_MUL_19", bcd_op_mul, "9999999999999999"                 ,       "9999999999"                 ,                     "9.999999998999999e+25" },
    { "BCD_MUL_20", bcd_op_mul, "9999999999999999"                 ,      "99999999999"                 ,                     "9.999999999899999e+26" },
    { "BCD_MUL_21", bcd_op_mul, "9999999999999999"                 ,     "999999999999"                 ,                     "9.999999999989999e+27" },
    { "BCD_MUL_22", bcd_op_mul, "9999999999999999"                 ,    "9999999999999"                 ,                     "9.999999999998999e+28" },
    { "BCD_MUL_23", bcd_op_mul, "9999999999999999"                 ,   "99999999999999"                 ,                     "9.999999999999899e+29" },
    { "BCD_MUL_24", bcd_op_mul, "9999999999999999"                 ,  "999999999999999"                 ,                     "9.999999999999989e+30" },
    { "BCD_MUL_25", bcd_op_mul, "9999999999999999"                 , "9999999999999999"                 ,                     "9.999999999999998e+31" },
    { "BCD_MUL_26", bcd_op_mul,                 ".000000000000001" ,                 ".000000000000001" ,                     "1e-30"                 }, // Very small numbers.
    { "BCD_MUL_27", bcd_op_mul,                 ".5"               ,                 ".2"               ,                     "0.1"                   }, // Insig zeroes in result.
    { "BCD_MUL_28", bcd_op_mul,               "75"                 ,               "28.2"               ,                 "2,115"                     }, // Whole * Fract = Whole.
    { "BCD_MUL_29", bcd_op_mul,              "428.225"             ,              "311"                 ,               "133,177.975"                 }, // Whole * Fract = Fract.
    { "BCD_MUL_30", bcd_op_mul,            "30000"                 ,              "200"                 ,             "6,000,000"                     }, // Many trailing zeroes.
    { "BCD_MUL_31", bcd_op_mul,                 ".009"             ,                 ".009"             ,                     "0.000081"              }, // Irrelevent carry.
    { "BCD_MUL_32", bcd_op_mul,                "9"                 ,                "9"                 ,                    "81"                     }, // Irrelevent carry.
    { "BCD_MUL_33", bcd_op_mul,              "370"                 ,                "3"                 ,                 "1,110"                     }, // Math test.
    { "BCD_MUL_34", bcd_op_mul,              "370"                 ,                "6"                 ,                 "2,220"                     }, // Math test.
    { "BCD_MUL_35", bcd_op_mul,              "370"                 ,                "9"                 ,                 "3,330"                     }, // Math test.
    { "BCD_MUL_36", bcd_op_mul,              "370"                 ,               "12"                 ,                 "4,440"                     }, // Math test.

    { "BCD_DIV_01", bcd_op_div,                "6"                 ,                "2"                 ,                     "3"                     }, // Simple div.
    { "BCD_DIV_02", bcd_op_div,              "246"                 ,                "3"                 ,                    "82"                     }, // Slightly fancier.
    { "BCD_DIV_03", bcd_op_div, "1234567890123456"                 ,               "32"                 ,    "38,580,246,566,358"                     }, // Slightly fancier.
    { "BCD_DIV_04", bcd_op_div,             "7890"                 ,             "3210"                 ,                     "2.457943925233645"     }, // Pos / Pos
    { "BCD_DIV_05", bcd_op_div,             "1234"                 ,               "32s"                ,                   "-38.5625"                }, // Pos / Neg
    { "BCD_DIV_06", bcd_op_div,            "97531s"                ,              "132"                 ,                  "-738.8712121212121"       }, // Neg / Pos
    { "BCD_DIV_07", bcd_op_div,       "2468013579s"                ,               "32s"                ,            "77,125,424.34375"               }, // Neg / Neg
    { "BCD_DIV_08", bcd_op_div, "9999999999999999"                 ,                 ".00234"           ,                     "4.273504273504273e+18" }, // Whole / <1
    { "BCD_DIV_09", bcd_op_div,                 ".45832"           ,               "32s"                ,                    "-0.0143225"             }, // <1 / Whole
    { "BCD_DIV_10", bcd_op_div, "9999999999999999"                 ,                 ".000000000000001" ,                     "9.999999999999999e+30" }, // Lrg / Sml
    { "BCD_DIV_11", bcd_op_div,                 ".000000000000001" , "9999999999999999"                 ,                     "1e-31"                 }, // Sml / Lrg
    { "BCD_DIV_12", bcd_op_div,       "8745963210"                 ,              "101"                 ,            "86,593,695.14851485"            }, //
    { "BCD_DIV_13", bcd_op_div,               "22"                 ,                "7"                 ,                     "3.142857142857143"     }, // Pi-ish
    { "BCD_DIV_14", bcd_op_div,                "2"                 ,                "1.414213562373095" ,                     "1.414213562373095"     }, // Square root of 2.
    { "BCD_DIV_15", bcd_op_div, "9999999999999999"                 , "7777777777777777"                 ,                     "1.285714285714286"     }, // 16 / 16 = 16 digits.
    { "BCD_DIV_16", bcd_op_div,                "3"                 , "1834944619757441"                 ,                     "1.634926726233605e-15" }, // Bug.
  };
  size_t bcd_math_test_size = (sizeof(math_tests) / sizeof(bcd_math_test));

  {
    int x;
    for(x = 0; x < bcd_math_test_size; x++)
    {
      bcd_math_test *t = &math_tests[x];
      const char *val1 = t->val1;
      const char *val2 = t->val2;
      printf("  %s: %s %s\n", t->name, val1, val2);

      bcd *obj1 = bcd_new();
      if((retcode = (obj1 != (bcd *) 0)) != true)                                             return false;
      while(*val1)
      {
        if((retcode = bcd_add_char(obj1, *val1++)) != true)                                   return false;
      }

      bcd *obj2 = bcd_new();
      if((retcode = (obj2 != (bcd *) 0)) != true)                                             return false;
      while(*val2)
      {
        if((retcode = bcd_add_char(obj2, *val2++)) != true)                                   return false;
      }

      if((retcode = t->func(obj1, obj2)) != true)                                             return false;
    
      char buf[1024];
      memset(buf, 0, sizeof(buf));
      if((retcode = bcd_to_str(obj1, buf, sizeof(buf))) != true)                              return false;

      if((retcode = (strcmp(t->result, buf) == 0)) != true)
      {
        printf("  %s: strcmp(%s, %s)\n", t->name, t->result, buf);
        return false;
      }

      if((retcode = bcd_delete(obj1)) != true)                                                return false;
      if((retcode = bcd_delete(obj2)) != true)                                                return false;
    }
  }
#endif // TEST_MATH_OPERATIONS

#ifdef TEST_SPECIAL
  {
    /* Specialty tests.  We're not fiddling around with creating a bunch of nifty
     * little objects.  We're building them manually and firing off the tests. */
    char buf[1024];

    printf("Divide by zero test.\n");
    bcd *o1 = bcd_new(), *o2 = bcd_new();
    if(bcd_sig_initialize(&o1->significand) != true)                                          return false;
    if(bcd_sig_initialize(&o2->significand) != true)                                          return false;
    if(bcd_sig_set_digit(&o1->significand, 0, 1) != true)                                     return false;
    o1->exponent = 1; o1->got_decimal_point = 0; o1->sign = false;
    o2->exponent = 1; o2->got_decimal_point = 0; o2->sign = false;
    if(bcd_op_div(o1, o2) != false)                                                           return false;

    printf("Mul very large and very small numbers.\n");
    int x;
    for(x = 0; x < BCD_NUM_DIGITS; x++) 
    {
      if(bcd_sig_set_digit(&o1->significand, x, 9) != true)                                   return false;
    }
    if(bcd_sig_set_digit(&o2->significand, 0, 1) != true)                                     return false;
    o1->exponent = 15; o1->got_decimal_point = 0; o1->sign = false;
    o2->exponent =  2; o2->got_decimal_point = 0; o2->sign = false;
    if(bcd_op_mul(o1, o2) != true)                                                            return false;
    memset(buf, 0, sizeof(buf));
    if((retcode = bcd_to_str(o1, buf, sizeof(buf))) != true)                                  return false;
    if((retcode = (strcmp("9.999999999999999e+17", buf) == 0)) != true)                       return false;

    printf("Now add a very small number.\n");
    if(bcd_sig_set_digit(&o2->significand, 0, 1) != true)                                     return false;
    o2->exponent =  1; o2->got_decimal_point = 0; o2->sign = false;
    if(bcd_op_add(o1, o2) != true)                                                            return false;
    memset(buf, 0, sizeof(buf));
    if((retcode = bcd_to_str(o1, buf, sizeof(buf))) != true)                                  return false;
    if((retcode = (strcmp("9.999999999999999e+17", buf) == 0)) != true)                       return false;

    printf("bcd_copy() and bcd_cmp().\n");
    if(bcd_sig_initialize(&o1->significand) != true)                                          return false;
    if(bcd_sig_set_digit(&o1->significand, 0, 1) != true)                                     return false;
    o1->exponent =  0; o1->got_decimal_point = 0; o1->sign = false;
    if(bcd_sig_initialize(&o2->significand) != true)                                          return false;
    if(bcd_sig_set_digit(&o2->significand, 0, 1) != true)                                     return false;
    o2->exponent =  0; o2->got_decimal_point = 0; o2->sign = false;
    if(bcd_cmp(o1, o2) != 0)                                                                  return false;
    o1->sign = true;
    if(bcd_cmp(o1, o2) != -1)                                                                 return false;
    o2->sign = true;
    if(bcd_cmp(o1, o2) !=  0)                                                                 return false;
    o1->sign = false;
    if(bcd_cmp(o1, o2) !=  1)                                                                 return false;
    for(x = 0; x < (BCD_NUM_DIGITS - 1); x++)
    {
      if(bcd_sig_set_digit(&o1->significand, x, 9) != true)                                   return false;
    }
    o1->exponent = 0; o1->sign = false;
    if(bcd_copy(o1, o2) != true)                                                              return false;
    if(bcd_cmp(o1, o2) !=  0)                                                                 return false;

    printf("bcd_import() and bcd_export().\n");
    int64_t exp;
    if(bcd_import(o1, 1000) != true)                                                          return false;
    if(bcd_export(o1, &exp) != true)                                                          return false;
    if(exp != 1000)                                                                           return false;
    if(bcd_import(o1, 0) != true)                                                             return false;
    if(bcd_add_char(o1, '5') != true)                                                         return false;
    if(bcd_add_char(o1, '0') != true)                                                         return false;
    if(bcd_add_char(o1, '0') != true)                                                         return false;
    if(bcd_export(o1, &exp) != true)                                                          return false;
    if(exp != 500)                                                                            return false;

    bcd_delete(o1); bcd_delete(o2);
  }
#endif // TEST_SPECIAL

  return retcode;
}

#endif // TEST

