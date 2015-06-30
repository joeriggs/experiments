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

#define max(a, b) ((a) > (b) ? (a) : (b))

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

typedef uint64_t significand_t;
#define BCD_MAX_DIGITS (sizeof(significand_t) * 2)
#define SIGNIFICAND_FMT "0x%016llX"

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
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/* This is a utility function.  You pass it a significand/exponent/sign, and it
 * creates a Sxxx.xxx ASCII string for you.  It does nothing fancy beyond that.
 * You have to make sure you pass it a BCD number that will fit that format.
 *
 * If the number requires more than 16 digits, then this function will fail.  It
 * expects the number to fit in a regular decimal notation.
 *
 * Input:
 *   significand = The 0 - 16 BCD digits.
 *
 *   exponent          = The exponent.  It tells us where to place the decimal
 *                       point.
 *
 *   char_count        = Once again, this is used in situations where the user
 *                       is typing in a number.  It tells us how many digits
 *                       they have typed.  It's particularly useful for
 *                       situations where they're typing zeroes after the
 *                       decimal point (ex, 0.00000).  We want to display all
 *                       the characters they've typed, even if it means we have
 *                       to display a bunch of insignificant zeroes.
 *                       (if char_count == 0), then ignore char_count and
 *                       got_decimal_point.
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
bcd_to_str_decimal(significand_t significand,
                   int16_t       exponent,
                   int           char_count,
                   bool          got_decimal_point,
                   uint8_t       sign,
                   char         *buf,
                   size_t        buf_size)
{
  bool retcode = false;

  /* We need a buffer, and the number has to fit within 16 digits. */
  if((buf != (char *) 0) && (buf_size > 0) &&
     (exponent <  16)    && (exponent > -16))
  {
    int buf_x = 0;

    do
    {
      DBG_PRINT("%s(): 0x%016llX, %d, %d, %d.\n", __func__,
                significand, exponent, got_decimal_point, sign);

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
       * 2. max((exponent + 1), (num non-zero digits in the significand)).
       *    2A. If exponent == 4, then there are at least 5 digits.
       *    2B. If there are 10 non-zero digits in the significand, then there
       *        are at least 10 digits.
       */
      int digit_count = char_count;
      if(digit_count == 0)
      {
        /* Count the non-zero digits in the significand. */
        int i;
        for(i = 0; i < BCD_MAX_DIGITS; i++)
        {
          significand_t mask = (0x000000000000000Fll << (i * 4));
          if(significand & mask)
          {
            break;
          }
        }
        digit_count = max((BCD_MAX_DIGITS - i), (exponent + 1));
      }

      int digit_position = (BCD_MAX_DIGITS - 1);
      for( ; digit_count > 0; digit_count--)
      {
        int shift = (digit_position-- * 4);
        char c = (significand >> shift) & 0xF;

        /* Insert the digit. */
        if(--buf_size == 0) { break; } else { buf[buf_x++] = (c | 0x30); }

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

/* Perform a straight addition of 2 significands that are guaranteed to not
 * overflow past the top of a significand_t.  The typical use of this method
 * would be to pass in 2 values that are exactly half of a significand_t in size
 * (for example, if a significand_t is 64-bits, the caller would pass in 2 32-bit
 * values to add).  This ensure the entire sum will fit in a significand_t val.
 * Then the caller can deal with overflow.
 *
 * Input:
 *   val1   = One of the significands.
 *
 *   val2   = One of the significands.
 *
 *   dst   = A pointer to the place to store the result.  Note that dst can
 *           point to val1 or val2.
 *
 * Output:
 *   true  = success.  *dst contains the sum.
 *   false = failure.  *dst is undefined.
 */
static bool
bcd_add_32bits(significand_t  val1,
               significand_t  val2,
               significand_t *dst)
{
  bool retcode = false;

  if(dst != (significand_t *) 0)
  {
//    DBG_PRINT("%s(%08llX %08llX)\n", __func__, val1, val2);
    significand_t t1 = val1 + 0x066666666ll;
    significand_t t2 = t1 + val2;
    significand_t t3 = t1 ^ val2;
    significand_t t4 = t2 ^ t3;
    significand_t t5 = ~t4 & 0x111111110ll;
    significand_t t6 = (t5 >> 2) | (t5 >> 3);
    *dst = t2 - t6;
//    DBG_PRINT("%s(): SUM %08llX\n", __func__, *dst);

    retcode = true;
  }

  return retcode;
}

/* Perform a straight addition of 2 significands.  No exponents or signs are
 * involved.  That needs to be taken care of somewhere else.  This method does
 * nothing more than add the 2 significands and returns the result.
 *
 * Input:
 *   val1      = One of the significands.
 *
 *   val2      = One of the significands.
 *
 *   dst       = A pointer to the place to store the result.  Note that dst can
 *               point to val1 or val2.
 *
 *   carry     = A pointer to a variable that will be set to true if a carry
 *               occurs at the end of the addition (ex. 8 + 5 = 13 is a carry).
 *               You can pass in a NULL.  Only pass in a non-NULL if you want
 *               to check for carry.  It's a manual operation in this function,
 *               so don't request it if you don't need it.
 *
 *   overflow  = A pointer to a variable that will receive the overflow if the
 *               sum doesn't fit in *dst.
 *
 * Output:
 *   true  = success.  *dst and *overflow contain the sum.
 *   false = failure.  *dst and *overflow are undefined.
 */
static bool
bcd_significand_add(significand_t  val1,
                    significand_t  val2,
                    significand_t *dst,
                    bool          *carry,
                    uint8_t       *overflow)
{
  bool retcode = false;

  if((dst != (significand_t *) 0) && (overflow != (uint8_t *) 0))
  {
//    DBG_PRINT("%s(" SIGNIFICAND_FMT ", " SIGNIFICAND_FMT ")\n", __func__, val1, val2);

    do
    {
      /* If we the caller requested a carry check, locate the high-order
       * significant digit before we add.  This will set carry_digit to the
       * highest non-zero digit in val1 and val2. */
      int carry_digit = 0;
      if(carry != (bool *) 0)
      {
        for(carry_digit = (BCD_MAX_DIGITS - 1); carry_digit > 0; carry_digit--)
        {
          int mask_shift = (carry_digit * 4);
          significand_t mask = 0xF;
          mask <<= mask_shift;
          if( ((val1 & mask) != 0) || ((val2 & mask) != 0) )
          {
            break;
          }
        }
      }

      significand_t a_lo = ((val1 >>  0) & 0x00000000FFFFFFFFll);
      significand_t a_hi = ((val1 >> 32) & 0x00000000FFFFFFFFll);
      significand_t b_lo = ((val2 >>  0) & 0x00000000FFFFFFFFll);
      significand_t b_hi = ((val2 >> 32) & 0x00000000FFFFFFFFll);

      /* Add the lower 32-bits. */
      significand_t tot_lo;
      if((retcode = bcd_add_32bits(a_lo, b_lo, &tot_lo)) != true) { break; }
//      DBG_PRINT("%s(1): " SIGNIFICAND_FMT "\n", __func__, tot_lo);
      *dst = (tot_lo & 0xFFFFFFFF);

      /* Carry to the high-order 32-bits. */
      significand_t tmp_carry = ((tot_lo >> 32) & 0xF);
      if((retcode = bcd_add_32bits(a_hi, tmp_carry, &a_hi)) != true) { break; }
//      DBG_PRINT("%s(2): " SIGNIFICAND_FMT "\n", __func__, a_hi);

      /* Add the high 32-bits. */
      significand_t high32;
      if((retcode = bcd_add_32bits(a_hi, b_hi, &high32)) != true) { break; }
//      DBG_PRINT("%s(3): " SIGNIFICAND_FMT "\n", __func__, high32);

      *dst |= ((high32 & 0x00000000FFFFFFFFll) << 32);
      *overflow = ((high32 >> 32) & 0xFF);
//      DBG_PRINT("%s(4): " SIGNIFICAND_FMT ": 0x%X\n", __func__, *dst, *overflow);

      /* If the caller requested a carry check, check now to see if there
       * was a carry at the end. */
      if(carry != (bool *) 0)
      {
        int mask_shift = (carry_digit + 1) * 4;
        significand_t mask = 0xFFFFFFFFFFFFFFFFll << mask_shift;
        *carry = ((*dst & mask) != 0ll) ? true : false;
      }

//      DBG_PRINT("%s(RES): " SIGNIFICAND_FMT ": 0x%X\n", __func__, *dst, *overflow);
      retcode = true;
    } while(0);
  }

  return retcode;
}

/* Perform a 10's complement on a BCD number.  This changes positives to
 * negatives and negatives to positives.
 *
 * Input:
 *   src  = The significand from the number.  We don't need the sign or
 *          exponent.
 *
 *   dst  = A pointer to the place to store the result.  Note that dst can
 *          point to src.
 *
 * Output:
 *   true  = success.  *dst contains the 10's complement of src.
 *   false = failure.  *dst is undefined.
 */
static bool
bcd_tens_complement(significand_t  src,
                    significand_t *dst)
{
  bool retcode = false;

  if(dst != (significand_t *) 0)
  {
//    DBG_PRINT("%s(): src: 0x%016llX.\n", __func__, src);
    src = 0x9999999999999999ll - src;

    significand_t one = 1;
    uint8_t overflow;
    retcode = bcd_significand_add(src, one, dst, NULL, &overflow);
//    DBG_PRINT("%s(): dst: 0x%016llX.\n", __func__, *dst);
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

  if((op1 != (significand_t *) 0) &&
     (exp1 != (int16_t *) 0)      &&
     (op2 != (significand_t *) 0) &&
     (exp2 != (int16_t *) 0))
  {
    /* If the exponents aren't the same, adjust the smaller number up to the other. */
    if(*exp1 > *exp2)
    {
      DBG_PRINT("Adjusting op2 so its exponent(%d) equals the op1 exponent (%d).\n", *exp2, *exp1);
      int shift = ((*exp1 - *exp2) * 4);
      *op2 >>= shift;
      *exp2 = *exp1;
    }
    else if(*exp1 < *exp2)
    {
      DBG_PRINT("Adjusting op1 so its exponent(%d) equals the op2 exponent (%d).\n", *exp1, *exp2);
      int shift = ((*exp2 - *exp1) * 4);
      *op1 >>= shift;
      *exp1 = *exp2;
    }

    retcode = true;
  }

  return retcode;
}

/* Shift the significand to remove leading zeroes.
 *
 * Input:
 *   op  = A pointer to the operand.
 *
 *   exp = A pointer to the exponent for the operand.
 *
 * Output:
 *   true  = success.  op and exp have been adjusted.
 *   false = failure.  The states of op and exp are undefined.
 */
static bool
bcd_shift_significand(significand_t *op,
                      int16_t       *exp)
{
  bool retcode = false;

  if((op != (significand_t *) 0) && (exp != (int16_t *) 0))
  {
    /* Clear out any leading zeroes in the significand. */
    while( (*op != 0) && ((*op & 0xF000000000000000ll) == 0) )
    {
      *op <<= 4;
      *exp = *exp - 1;
    }

    retcode = true;
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
      /* Start with the raw significands. */
      significand_t a = op1->significand;
      significand_t b = op2->significand;
      DBG_PRINT("%s(): " SIGNIFICAND_FMT " + " SIGNIFICAND_FMT "\n", __func__, a, b);

      /* If the exponents aren't the same, adjust the smaller number up to the other. */
      if((retcode = bcd_make_exponents_equal(&a, &op1->exponent, &b, &op2->exponent)) != true) break;

      /* If either number is negative, do a 10's complement before the addition
       * step. */
      if(op1->sign == true) { if((retcode = bcd_tens_complement(a, &a)) == false) break; }
      if(op2->sign == true) { if((retcode = bcd_tens_complement(b, &b)) == false) break; }
      DBG_PRINT("%s(1): " SIGNIFICAND_FMT ", " SIGNIFICAND_FMT "\n", __func__, a, b);

      uint8_t overflow;
      if((retcode = bcd_significand_add(a, b, &op1->significand, NULL, &overflow)) != true) { break; }
      DBG_PRINT("%s(2): " SIGNIFICAND_FMT "\n", __func__, op1->significand);

      /* If exactly one of the operands is negative:
       * - If we have overflow, then the result is positive.
       * - If we have no overflow, then the result is negative.
       */
      if(op1->sign != op2->sign)
      {
        if(overflow == 0)
        {
          if((retcode = bcd_tens_complement(op1->significand, &op1->significand)) == false) break;
        }
        op1->sign = (overflow == 0) ? true : false;
      }

      /* Otherwise the signs are the same.  The sum will have the same sign. */
      else
      {
        /* If the result is negative, do 10s complement now. */
        if(op1->sign == true)
        {
          if((retcode = bcd_tens_complement(op1->significand, &op1->significand)) == false) break;
          overflow ^= 1;
        }

        /* If overflow, shift significand, insert overflow, bump exponent. */
        if(overflow != 0)
        {
          significand_t overflow_big = overflow;
          op1->significand >>= 4;
          op1->significand |= (overflow_big << 60);
          op1->exponent++;
        }
      }
      DBG_PRINT("%s(3): " SIGNIFICAND_FMT " %d %d.\n", __func__, op1->significand, op1->exponent, op1->sign);

      /* Clear out any leading zeroes in the significand. */
      if((retcode = bcd_shift_significand(&op1->significand, &op1->exponent)) != true) break;
      DBG_PRINT("%s(4): " SIGNIFICAND_FMT " %d %d.\n", __func__, op1->significand, op1->exponent, op1->sign);

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
    do
    {
      /* Start with the raw significands. */
      significand_t a = op1->significand;
      significand_t b = op2->significand;
      DBG_PRINT("%s() BEGIN: " SIGNIFICAND_FMT " - " SIGNIFICAND_FMT "\n", __func__, a, b);

      /* If the exponents aren't the same, adjust the smaller number up to the other. */
      if((retcode = bcd_make_exponents_equal(&a, &op1->exponent, &b, &op2->exponent)) != true) break;

      /* 10s complement (as required):
       * POS - POS  = 10's complement b.
       * POS - NEG  = NO 10's complement.
       * NEG - POS  = NO 10's complement.
       * NEG - NEG  = 10's complement a.
       */
      if((op1->sign == false) && (op2->sign == false)) { if((retcode = bcd_tens_complement(b, &b)) == false) break; }
      if((op1->sign ==  true) && (op2->sign ==  true)) { if((retcode = bcd_tens_complement(a, &a)) == false) break; }
      DBG_PRINT("%s() TENS: " SIGNIFICAND_FMT ", " SIGNIFICAND_FMT "\n", __func__, a, b);

      uint8_t overflow;
      if((retcode = bcd_significand_add(a, b, &op1->significand, NULL, &overflow)) != true) break;
      DBG_PRINT("%s(): RESULT " SIGNIFICAND_FMT " CARRY %d.\n", __func__, op1->significand, overflow);

      /* 10s complement (as required) and set the result sign:
       * POS - POS  = Sign is defined by overflow.
       * POS - NEG  = Sign is positive.
       * NEG - POS  = Sign is negative.
       * NEG - NEG  = Sign is defined by overflow.
       */
      if(op1->sign == op2->sign)
      {
        op1->sign = (overflow != 0) ? false : true;
        if(op1->sign == true)
        {
          if((retcode = bcd_tens_complement(op1->significand, &op1->significand)) != true) break;
          DBG_PRINT("%s(): NEGATIVE " SIGNIFICAND_FMT ".\n", __func__, op1->significand);
        }
      }
      else
      {
        op1->sign = (op1->sign == false) ? false : true;
      }

      /* Clear out any leading zeroes in the significand. */
      if((retcode = bcd_shift_significand(&op1->significand, &op1->exponent)) != true) break;
      DBG_PRINT("%s() SHIFT: " SIGNIFICAND_FMT " %d %d.\n", __func__, op1->significand, op1->exponent, op1->sign);

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
      /* Normalize both numbers before we begin.  We need the exponents. */
      if((retcode = bcd_shift_significand(&op1->significand, &op1->exponent)) != true) break;
      if((retcode = bcd_shift_significand(&op2->significand, &op2->exponent)) != true) break;

      /* Start with the raw significands. */
      significand_t a = op1->significand;
      significand_t b = op2->significand;
      DBG_PRINT("%s() BEGIN: " SIGNIFICAND_FMT " * " SIGNIFICAND_FMT "\n", __func__, a, b);

      /* We'll store the result here. */
      significand_t result_hi = 0ll, result_lo = 0ll;

      bool carry;

      int b_digit;
      for(b_digit = 0; b_digit < BCD_MAX_DIGITS; b_digit++)
      {
        int a_digit;
        for(a_digit = 0; a_digit < BCD_MAX_DIGITS; a_digit++)
        {
          /* Get the 2 digits, and then multiply them.  This creates a hex
           * result.  Convert the hex result into a base-10 result (res) and
           * remainder (rem). */
          uint8_t a_byte = (a >> (a_digit * 4)) & 0xF;
          uint8_t b_byte = (b >> (b_digit * 4)) & 0xF;
          if((a_byte == 0) || (b_byte == 0)) continue;
          uint8_t prod = a_byte * b_byte;
          significand_t result = (prod / 10);
          significand_t remain = (prod % 10);

          /* 1. Set remain_digit, result_digit, and carry_digit to the position
           *    (relative to the right-hand edge of result_hi:result_lo) where
           *    the result, remainder, and carry should be added into the total.
           * 
           * 2. Set remain_sig, result_sig, and carry_sig to point to either
           *    result_hi or result_lo, depending on where they fall in the
           *    total result space.
           *
           * 3. Adjust remain_digit, result_digit, and carry_digit to their
           *    position (relative to either result_hi or result_lo).
           *
           * 4. Shift the result and remain to their rightful position within
           *    result_hi or result_lo.
           */
          int remain_digit = b_digit + a_digit;
          int result_digit = remain_digit + 1;
          significand_t *remain_sig = (remain_digit < BCD_MAX_DIGITS) ? &result_lo : &result_hi;
          significand_t *result_sig = (result_digit < BCD_MAX_DIGITS) ? &result_lo : &result_hi;
          remain_digit %= BCD_MAX_DIGITS;
          result_digit %= BCD_MAX_DIGITS;
          remain <<= (remain_digit * 4);
          result <<= (result_digit * 4);

          /* Now we're ready to add remain and result to the result_hi:result_lo total. */
          uint8_t overflow;
          if((remain != 0) && (retcode = bcd_significand_add(*remain_sig, remain, remain_sig, &carry, &overflow)) != true) break;
          if((overflow != 0) && (remain_sig == &result_lo))
          {
            significand_t overflo_val = 1;
            if((result != 0ll) && (retcode = bcd_significand_add(result_hi, overflo_val, &result_hi, NULL, &overflow)) != true) break;
          }
          if((result != 0ll) && (retcode = bcd_significand_add(*result_sig, result, result_sig, &carry, &overflow)) != true) break;
          if((overflow != 0) && (result_sig == &result_lo))
          {
            significand_t overflo_val = 1;
            if((result != 0ll) && (retcode = bcd_significand_add(result_hi, overflo_val, &result_hi, NULL, &overflow)) != true) break;
          }
          DBG_PRINT("%s(): RES: %016llX:%016llX\n", __func__, result_hi, result_lo);
        }
      }
      if(retcode != true) break;

      /* Shift the result. */
      op1->significand = result_lo;
      while(result_hi != 0ll)
      {
        op1->significand >>= 4;
        op1->significand |= (result_hi << ((BCD_MAX_DIGITS - 1) * 4));
        result_hi >>= 4;
      }

      /* If the result is zero, then set the exponent to zero and leave. */
      if(op1->significand == 0)
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
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Division is BINARY.
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
      significand_t dividend = op1->significand;
      significand_t divisor  = op2->significand;
      DBG_PRINT("%s() BEGIN: " SIGNIFICAND_FMT " / " SIGNIFICAND_FMT "\n", __func__, dividend, divisor);

      /* Check for divide by zero. */
      if(divisor == 0ll) break;

      op1->significand = 0ll;

      /* This mask will help us locate the significant divisor digits. */
      significand_t mask = (0xFll << ((BCD_MAX_DIGITS - 1) * 4));
      while((divisor & mask) != divisor)
      {
        mask >>= 4;
        mask |= (0xFll << ((BCD_MAX_DIGITS - 1) * 4));
      }
      DBG_PRINT("%s() MASK: Divisor " SIGNIFICAND_FMT ": Mask " SIGNIFICAND_FMT "\n", __func__, divisor, mask);

      /* This identifies the digit position (relative to 0 on the right) where
       * we are currently calculating the quotient. */
     significand_t quotient_one = (1ll << ((BCD_MAX_DIGITS - 1) * 4));

      /* Loop here until we're done with the division. */
      while((dividend != 0ll) && (divisor != 0ll))
      {
        /* Calculate a quotient digit.  Keep subtracting and looping. */
        while((divisor & mask) <= (dividend & mask))
        {
          op1->significand += quotient_one;

          uint8_t overflow;
          significand_t divisor_tens;
          if((retcode = bcd_tens_complement(divisor, &divisor_tens)) == false) break;
          if((retcode = bcd_significand_add(dividend, divisor_tens, &dividend, NULL, &overflow)) != true) break;
        }

        divisor      >>= 4;
        quotient_one >>= 4;
        mask         >>= 4;
        mask         |= (0xFll << ((BCD_MAX_DIGITS - 1) * 4));
      }
      DBG_PRINT("%s() RESULT: " SIGNIFICAND_FMT "\n", __func__, op1->significand);

      /* Set the exponent, and then adjust to account for any leading zeroes. */
      op1->exponent -= op2->exponent;
      mask = (0xFll << ((BCD_MAX_DIGITS - 1) * 4));
      while((op1->significand != 0ll) && ((op1->significand & mask) == 0))
      {
        op1->significand <<= 4;
        op1->exponent--;
      }

      /* Set the sign. */
      op1->sign = (op1->sign == op2->sign) ? false : true;

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

  if((op1 != (bcd *) 0) && (op2 != (bcd *) 0))
  {

  }
    
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
    /* Start with zero (+0 * 10^0 = 0). */
    this->significand       = 0;
    this->exponent          = 0;
    this->sign              = 0;

    /* These only come into play if we start receiving characters via
     * bcd_add_char(). */
    this->got_decimal_point = false;
    this->char_count        = 0;
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

  /* If it's a decimal point, prepare to start doing decimal math.  If we
   * already got a decimal point, then this one is silently dropped. */
  if(c == '.')
  {
    /* If this is the first significant character, reserve a leading zero. */
    if(this->char_count == 0)
    {
      this->char_count = 1;
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
    if((c == 0) && (this->got_decimal_point == false) && (this->significand == 0ll))
    {
    }

    /* If the significand is already full, then silently drop the character. */
    else if(this->char_count < BCD_MAX_DIGITS)
    {
      if((this->got_decimal_point == false) && (this->significand != 0ll))
      {
        this->exponent++;
      }

      this->char_count++;

      int shift_count = ((BCD_MAX_DIGITS - this->char_count) * 4);
      this->significand |= (((significand_t) c) << shift_count);
      DBG_PRINT("%s(): 0x%016llX %d\n", __func__, this->significand, this->exponent);
    }

    retcode = true;
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
    DBG_PRINT("%s(): 0x%016llX, %d, %d, %d.\n", __func__,
              this->significand, this->exponent, this->got_decimal_point, this->sign);

    int16_t exp = this->exponent;
    if((exp > -16) && (exp < 16))
    {
      retcode = bcd_to_str_decimal(this->significand,
                                   this->exponent,
                                   this->char_count,
                                   this->got_decimal_point,
                                   this->sign,
                                   buf,
                                   buf_size);
    }
    /* Need to use scientific notation (1.234e18). */
    else
    {
      retcode = bcd_to_str_decimal(this->significand,
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

/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#ifdef TEST
bool
bcd_test(void)
{
  bool retcode = false;

  /* Loop through some decimal numbers.  This tests the basic functionality of
   * the bcd class.  We're checking to make sure it can handle any type of
   * decimal number that we might throw at it. */
  typedef struct bcd_test {
    const char  *name;
    const char  *src;
    const char  *dst;
  } bcd_test;
  bcd_test tests[] = {
    { "BCD_01",          ""                 ,         "0"                 }, // A blank object.
    { "BCD_02",         "1"                 ,         "1"                 }, // A single-digit number (exp = 0).
    { "BCD_03",       "123"                 ,       "123"                 }, // Simple integer value (exp > 0).
    { "BCD_04",       "123."                ,       "123."                }, // User just entered a decimal point.
    { "BCD_05",    "123000"                 ,    "123000"                 }, // Integer with trailing zeroes.
    { "BCD_06", "000123000"                 ,    "123000"                 }, // Insignificant leading zeroes.
    { "BCD_07",       "123.456"             ,       "123.456"             }, // Simple floating point value.
    { "BCD_08",       "123.456000"          ,       "123.456000"          }, // Insignificant trailing zeroes.
    { "BCD_09",       "123.456007"          ,       "123.456007"          }, // Significant zeroes in middle of the decimal.
    { "BCD_10",       "000.000123"          ,         "0.000123"          }, // Insignificant and significant zeroes.
    { "BCD_11",       "000.0123S"           ,        "-0.0123"            }, // Negative number.
    { "BCD_12",          ".000000000000000" ,         "0.000000000000000" }, // No significant digit.
    { "BCD_13",          ".000000000000001" ,         "0.000000000000001" }, // One significant digit.
    { "BCD_14",          ".000123"          ,         "0.000123"          },
  };
  size_t bcd_test_size = (sizeof(tests) / sizeof(bcd_test));

  int x;
  for(x = 0; x < bcd_test_size; x++)
  {
    bcd_test *t = &tests[x];
    printf("  %s: '%s'.\n", t->name, t->src);

    bcd *this = bcd_new();
    if((retcode = (this != (bcd *) 0)) != true)                      return false;

    const char *src = t->src;
    while(*src)
    {
      if((retcode = bcd_add_char(this, *src++)) != true)             return false;
    }

    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if((retcode = bcd_to_str(this, buf, sizeof(buf))) != true)       return false;

    DBG_PRINT("strcmp(%s, %s)\n", t->dst, buf);
    if((retcode = (strcmp(t->dst, buf) == 0)) != true)               return false;

    if((retcode = bcd_delete(this)) != true)                         return false;
    this = (bcd *) 0;
  }
  
  /* Math operations. */
  typedef struct bcd_math_test {
    const char  *name;
    const char  *val1;
    const char  *val2;
    bool (*func)(bcd *val1, bcd *val2);
    const char  *result;
  } bcd_math_test;
  bcd_math_test math_tests[] = {
    { "BCD_ADD_01",                "1"                 ,                "2"                 , bcd_op_add,                "3"                     }, // Debug
    { "BCD_ADD_02",         "99999999"                 ,                "1"                 , bcd_op_add,        "100000000"                     }, // 33-bits
    { "BCD_ADD_03",        "999999999"                 ,                "1"                 , bcd_op_add,       "1000000000"                     }, // Carry.
    { "BCD_ADD_04", "1234567890123456"                 , "9876543210987654"                 , bcd_op_add,                "1.111111110111111e+16" }, // 17 digits
    { "BCD_ADD_05",                 ".1234567890123456", "9876543210987654"                 , bcd_op_add, "9876543210987654"                     }, // 16.0 + 0.16 digits.
    { "BCD_ADD_06",             "1234s"                ,             "4321"                 , bcd_op_add,             "3087"                     }, // 1st num neg.
    { "BCD_ADD_07",             "8766"                 ,             "4321"                 , bcd_op_add,            "13087"                     }, // Like previous, but pos.
    { "BCD_ADD_08",              "123"                 ,             "1234"                 , bcd_op_add,             "1357"                     }, // Pos + Pos.
    { "BCD_ADD_09",              "456s"                ,              "123"                 , bcd_op_add,             "-333"                     }, // Neg + Pos = Neg.
    { "BCD_ADD_10",              "456s"                ,             "1234"                 , bcd_op_add,              "778"                     }, // Neg + Pos = Pos.
    { "BCD_ADD_11",              "789"                 ,             "1234s"                , bcd_op_add,             "-445"                     }, // Pos + Neg = Neg.
    { "BCD_ADD_12",              "789"                 ,              "123s"                , bcd_op_add,              "666"                     }, // Pos + Neg = Pos.
    { "BCD_ADD_13",              "202s"                ,             "1234s"                , bcd_op_add,            "-1436"                     }, // Neg + Neg.
    { "BCD_ADD_14",             "9990s"                ,             "1234s"                , bcd_op_add,           "-11224"                     }, // Neg with carry.
    { "BCD_ADD_15",               "10.5"               ,                 ".5"               , bcd_op_add,               "11"                     }, // decimal to whole.
    { "BCD_ADD_16",                 ".1111111111111111",                 ".1111111111111111", bcd_op_add,                "0.222222222222222"     }, // No carry, truncate.
    { "BCD_ADD_17",             "1000"                 ,             "1000"                 , bcd_op_add,             "2000"                     }, // Trailing zeroes.
    { "BCD_ADD_18",                 ".00001"           ,                 ".00001"           , bcd_op_add,                "0.00002"               }, // Significant zeroes.
    { "BCD_ADD_19",                "1.0000134s"        ,                 ".045"             , bcd_op_add,               "-0.9550134"             },

    { "BCD_SUB_01",                "5"                 ,                "2"                 , bcd_op_sub,                "3"                     }, // Debug.
    { "BCD_SUB_02",                "0"                 ,                "1"                 , bcd_op_sub,               "-1"                     }, // Neg num.
    { "BCD_SUB_03",            "12345"                 ,             "1234"                 , bcd_op_sub,            "11111"                     }, // Pos - Pos = Pos.
    { "BCD_SUB_04",            "54321"                 ,            "91234"                 , bcd_op_sub,           "-36913"                     }, // Pos - Pos = Neg.
    { "BCD_SUB_05",            "12345"                 ,              "123.4s"              , bcd_op_sub,            "12468.4"                   }, // Pos - Neg = Pos.
    { "BCD_SUB_06",              "432.1s"              ,                "7.5678"            , bcd_op_sub,             "-439.6678"                }, // Neg - Pos = Neg.
    { "BCD_SUB_07",             "1225s"                ,               "34.95s"             , bcd_op_sub,            "-1190.05"                  }, // Neg - Neg = Neg.
    { "BCD_SUB_08", "1111111111111111s"                , "1234567890123456s"                , bcd_op_sub,  "123456779012345"                     }, // Neg - Neg = Pos.

    { "BCD_MUL_01",                "3"                 ,                "2"                 , bcd_op_mul,                "6"                     }, // Debug.
    { "BCD_MUL_02",             "4567"                 ,            "56789"                 , bcd_op_mul,        "259355363"                     }, // Lots of carry.
    { "BCD_MUL_03",                "1"                 ,                "0"                 , bcd_op_mul,                "0"                     }, // Non-0 * 0 = 0.
    { "BCD_MUL_04",                "0"                 ,                "8"                 , bcd_op_mul,                "0"                     }, // 0 * Non-0 = 0.
    { "BCD_MUL_05",            "87878"                 ,             "4539.123"             , bcd_op_mul,        "398889050.994"                 }, // Pos * Pos = Pos.
    { "BCD_MUL_06",            "13579.2468"            ,                 ".8579s"           , bcd_op_mul,           "-11649.63582972"            }, // Pos * Neg = Neg.
    { "BCD_MUL_07",                "1.0000134s"        ,                 ".045"             , bcd_op_mul,               "-0.045000603"           }, // Neg * Pos = Neg.
    { "BCD_MUL_08",       "5579421358s"                ,               "42s"                , bcd_op_mul,     "234335697036"                     }, // Neg * Neg = Pos.
    { "BCD_MUL_09",               "13.57900000"        ,             "8700.0000"            , bcd_op_mul,          "118137.3"                    }, // Insignificant zeroes.
    { "BCD_MUL_10", "9999999999999999"                 ,                "9"                 , bcd_op_mul,                "8.999999999999999e+16" }, // Very large numbers. 
    { "BCD_MUL_11", "9999999999999999"                 ,               "99"                 , bcd_op_mul,                "9.899999999999999e+17" },
    { "BCD_MUL_12", "9999999999999999"                 ,              "999"                 , bcd_op_mul,                "9.989999999999999e+18" },
    { "BCD_MUL_13", "9999999999999999"                 ,             "9999"                 , bcd_op_mul,                "9.998999999999999e+19" },
    { "BCD_MUL_14", "9999999999999999"                 ,            "99999"                 , bcd_op_mul,                "9.999899999999999e+20" },
    { "BCD_MUL_15", "9999999999999999"                 ,           "999999"                 , bcd_op_mul,                "9.999989999999999e+21" },
    { "BCD_MUL_16", "9999999999999999"                 ,          "9999999"                 , bcd_op_mul,                "9.999998999999999e+22" },
    { "BCD_MUL_17", "9999999999999999"                 ,         "99999999"                 , bcd_op_mul,                "9.999999899999999e+23" },
    { "BCD_MUL_18", "9999999999999999"                 ,        "999999999"                 , bcd_op_mul,                "9.999999989999999e+24" },
    { "BCD_MUL_19", "9999999999999999"                 ,       "9999999999"                 , bcd_op_mul,                "9.999999998999999e+25" },
    { "BCD_MUL_20", "9999999999999999"                 ,      "99999999999"                 , bcd_op_mul,                "9.999999999899999e+26" },
    { "BCD_MUL_21", "9999999999999999"                 ,     "999999999999"                 , bcd_op_mul,                "9.999999999989999e+27" },
    { "BCD_MUL_22", "9999999999999999"                 ,    "9999999999999"                 , bcd_op_mul,                "9.999999999998999e+28" },
    { "BCD_MUL_23", "9999999999999999"                 ,   "99999999999999"                 , bcd_op_mul,                "9.999999999999899e+29" },
    { "BCD_MUL_24", "9999999999999999"                 ,  "999999999999999"                 , bcd_op_mul,                "9.999999999999989e+30" },
    { "BCD_MUL_25", "9999999999999999"                 , "9999999999999999"                 , bcd_op_mul,                "9.999999999999998e+31" },
    { "BCD_MUL_26",                 ".000000000000001" ,                 ".000000000000001" , bcd_op_mul,                "1e-30"                 }, // Very small numbers.
    { "BCD_MUL_27",                 ".5"               ,                 ".2"               , bcd_op_mul,                "0.1"                   }, // Insig zeroes in result.
    { "BCD_MUL_28",               "75"                 ,               "28.2"               , bcd_op_mul,             "2115"                     }, // Whole * Fract = Whole.
    { "BCD_MUL_29",              "428.225"             ,              "311"                 , bcd_op_mul,           "133177.975"                 }, // Whole * Fract = Fract.
    { "BCD_MUL_30",            "30000"                 ,              "200"                 , bcd_op_mul,          "6000000"                     }, // Many trailing zeroes.

    { "BCD_DIV_01",                "6"                 ,                "2"                 , bcd_op_div,                "3"                     }, // Simple div.
    { "BCD_DIV_02",              "246"                 ,                "3"                 , bcd_op_div,               "82"                     }, // Slightly fancier.
    { "BCD_DIV_03", "1234567890123456"                 ,               "32"                 , bcd_op_div,   "38580246566358"                     }, // Slightly fancier.
    { "BCD_DIV_04",             "7890"                 ,             "3210"                 , bcd_op_div,                "2.457943925233645"     }, // Pos / Pos
    { "BCD_DIV_05",             "1234"                 ,               "32s"                , bcd_op_div,              "-38.5625"                }, // Pos / Neg
    { "BCD_DIV_06",            "97531s"                ,              "132"                 , bcd_op_div,             "-738.8712121212121"       }, // Neg / Pos
    { "BCD_DIV_07",       "2468013579s"                ,               "32s"                , bcd_op_div,         "77125424.34375"               }, // Neg / Neg
    { "BCD_DIV_08", "9999999999999999"                 ,                 ".00234"           , bcd_op_div,                "4.273504273504273e+18" }, // Whole / <1
    { "BCD_DIV_09",                 ".45832"           ,               "32s"                , bcd_op_div,               "-0.0199269565217391"    }, // <1 / Whole
    { "BCD_DIV_10", "9999999999999999"                 ,                 ".000000000000001" , bcd_op_div,                "9.999999999999999e+30" }, // Lrg / Sml
    { "BCD_DIV_11",                 ".000000000000001" , "9999999999999999"                 , bcd_op_div,                "1.e-31"                }, // Sml / Lrg
    { "BCD_DIV_12",      "8745963210"                  ,              "101"                 , bcd_op_div,         "86593694.14851485"            }, //
    { "BCD_DIV_13",              "22"                  ,                "7"                 , bcd_op_div,                "3.142857142857143"     }, // Pi
    { "BCD_DIV_14",               "2"                  ,                "1.414213562373095" , bcd_op_div,                "1.414213562373095"     }, // Square root of 2.
    { "BCD_DIV_15", "9999999999999999"                 , "7777777777777777"                 , bcd_op_div,                "1.285714285714286"     }, // 16 / 16 = 16 digits.
  };
  size_t bcd_math_test_size = (sizeof(math_tests) / sizeof(bcd_math_test));

  for(x = 0; x < bcd_math_test_size; x++)
  {
    bcd_math_test *t = &math_tests[x];
    const char *val1 = t->val1;
    const char *val2 = t->val2;
    printf("  %s: %s %s\n", t->name, val1, val2);

    bcd *obj1 = bcd_new();
    if((retcode = (obj1 != (bcd *) 0)) != true)                      return false;
    while(*val1)
    {
      if((retcode = bcd_add_char(obj1, *val1++)) != true)            return false;
    }

    bcd *obj2 = bcd_new();
    if((retcode = (obj2 != (bcd *) 0)) != true)                      return false;
    while(*val2)
    {
      if((retcode = bcd_add_char(obj2, *val2++)) != true)            return false;
    }

    if((retcode = t->func(obj1, obj2)) != true)                      return false;
    
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if((retcode = bcd_to_str(obj1, buf, sizeof(buf))) != true)       return false;

    if((retcode = (strcmp(t->result, buf) == 0)) != true)
    {
      printf("  %s: strcmp(%s, %s)\n", t->name, t->result, buf);
      return false;
    }

    if((retcode = bcd_delete(obj1)) != true)                         return false;
    if((retcode = bcd_delete(obj2)) != true)                         return false;
  }

  /* Specialty tests.  We're not fiddling around with creating a bunch of nifty
   * little objects.  We're building them manually and firing off the tests. */
  char buf[1024];

  printf("Divide by zero test.\n");
  bcd *o1 = bcd_new(), *o2 = bcd_new();
  o1->significand = 0x01; o1->exponent = 1; o1->got_decimal_point = 0; o1->sign = 0;
  o2->significand = 0x00; o2->exponent = 1; o2->got_decimal_point = 0; o2->sign = 0;
  if(bcd_op_div(o1, o2) != false)                                    return false;

  printf("Mul very large and very small numbers.\n");
  o1->significand = 0x9999999999999999ll; o1->exponent = 15; o1->got_decimal_point = 0; o1->sign = 0;
  o2->significand = 0x0000000000000001ll; o2->exponent =  2; o2->got_decimal_point = 0; o2->sign = 0;
  if(bcd_op_mul(o1, o2) != true)                                     return false;
  memset(buf, 0, sizeof(buf));
  if((retcode = bcd_to_str(o1, buf, sizeof(buf))) != true)           return false;
  if((retcode = (strcmp("9.999999999999999e17", buf) == 0)) != true) return false;

  printf("Now add a very small number.\n");
  o2->significand = 0x0000000000000001ll; o2->exponent =  1; o2->got_decimal_point = 0; o2->sign = 0;
  if(bcd_op_add(o1, o2) != true)                                     return false;
  memset(buf, 0, sizeof(buf));
  if((retcode = bcd_to_str(o1, buf, sizeof(buf))) != true)           return false;
  if((retcode = (strcmp("9.999999999999999e17", buf) == 0)) != true) return false;
  bcd_delete(o1); bcd_delete(o2);

  return retcode;
}

#endif // TEST

