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

  /* This is the significand. */
  significand_t significand;

  /* This is the exponent.  It's >= 0 if (|number| >= 0) and it's < 0 if
   * (|number| < 1). */
  int16_t exponent;

  /* This is the sign. */
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
 * carry past the top of a significand_t.  The typical use of this method would
 * be to pass in 2 values that are exactly half of a significand_t in size (for
 * example, if a significand_t is 64-bits, the caller would pass in 2 32-bit
 * values to add).  This ensure the entire sum will fit in a significand_t val.
 * Then the caller can deal with carry.
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
    DBG_PRINT("%s(" SIGNIFICAND_FMT ", " SIGNIFICAND_FMT ")\n", __func__, val1, val2);
    significand_t t1 = val1 + 0x066666666ll;
    significand_t t2 = t1 + val2;
    significand_t t3 = t1 ^ val2;
    significand_t t4 = t2 ^ t3;
    significand_t t5 = ~t4 & 0x111111110ll;
    significand_t t6 = (t5 >> 2) | (t5 >> 3);
    *dst = t2 - t6;
    DBG_PRINT("%s(): SUM " SIGNIFICAND_FMT "\n", __func__, *dst);

    retcode = true;
  }

  return retcode;
}

/* Perform a straight addition of 2 significands.  No exponents or signs are
 * involved.  That needs to be taken care of somewhere else.  This method does
 * nothing more than add the 2 significands and returns the result.
 *
 * Input:
 *   val1   = One of the significands.
 *
 *   val2   = One of the significands.
 *
 *   dst   = A pointer to the place to store the result.  Note that dst can
 *           point to val1 or val2.
 *
 *   carry = A pointer to a variable that will receive the carry if the sum
 *           doesn't fit in *dst.
 *
 * Output:
 *   true  = success.  *dst and *carry contain the sum.
 *   false = failure.  *dst and *carry are undefined.
 */
static bool
bcd_significand_add(significand_t  val1,
                    significand_t  val2,
                    significand_t *dst,
                    uint8_t       *carry)
{
  bool retcode = false;

  if((dst != (significand_t *) 0) && (carry != (uint8_t *) 0))
  {
    DBG_PRINT("%s(" SIGNIFICAND_FMT ", " SIGNIFICAND_FMT ")\n", __func__, val1, val2);

    do
    {
      significand_t a_lo = ((val1 >>  0) & 0x00000000FFFFFFFFll);
      significand_t a_hi = ((val1 >> 32) & 0x00000000FFFFFFFFll);
      significand_t b_lo = ((val2 >>  0) & 0x00000000FFFFFFFFll);
      significand_t b_hi = ((val2 >> 32) & 0x00000000FFFFFFFFll);

      /* Add the lower 32-bits. */
      significand_t tot_lo;
      if((retcode = bcd_add_32bits(a_lo, b_lo, &tot_lo)) != true) { break; }
      DBG_PRINT("%s(1): " SIGNIFICAND_FMT "\n", __func__, tot_lo);
      *dst = (tot_lo & 0xFFFFFFFF);

      /* Carry to the high-order 32-bits. */
      significand_t carry_tmp = ((tot_lo >> 32) & 0xF);
      if((retcode = bcd_add_32bits(a_hi, carry_tmp, &a_hi)) != true) { break; }
      DBG_PRINT("%s(2): " SIGNIFICAND_FMT "\n", __func__, a_hi);

      /* Add the high 32-bits. */
      significand_t high32;
      if((retcode = bcd_add_32bits(a_hi, b_hi, &high32)) != true) { break; }
      DBG_PRINT("%s(3): " SIGNIFICAND_FMT "\n", __func__, high32);

      *dst |= ((high32 & 0x00000000FFFFFFFFll) << 32);
      *carry = ((high32 >> 32) & 0xFF);
      DBG_PRINT("%s(4): " SIGNIFICAND_FMT ": 0x%X\n", __func__, *dst, *carry);

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
    DBG_PRINT("%s(): src: 0x%016llX.\n", __func__, src);
    src = 0x9999999999999999ll - src;

    significand_t one = 1;
    uint8_t carry;
    retcode = bcd_significand_add(src, one, dst, &carry);
    DBG_PRINT("%s(): dst: 0x%016llX.\n", __func__, *dst);
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

      /* If the exponents aren't the same, adjust the smaller number up to the other. */
      if(op1->exponent > op2->exponent)
      {
        DBG_PRINT("Adjusting op2 so its exponent(%d) equals the op1 exponent (%d).\n", op2->exponent, op1->exponent);
        int shift = ((op1->exponent - op2->exponent) * 4);
        b >>= shift;
      }
      else if(op1->exponent < op2->exponent)
      {
        DBG_PRINT("Adjusting op1 so its exponent(%d) equals the op2 exponent (%d).\n", op1->exponent, op2->exponent);
        int shift = ((op2->exponent - op1->exponent) * 4);
        a >>= shift;

        /* The result will be shipped back in op1.  So adjust op1 now to reflect
         * the shift results. */
        op1->exponent = op2->exponent;
      }

      /* If either number is negative, do a 10's complement before the addition
       * step. */
      if(op1->sign == true) { if((retcode = bcd_tens_complement(a, &a)) == false) break; }
      if(op2->sign == true) { if((retcode = bcd_tens_complement(b, &b)) == false) break; }
      DBG_PRINT("%s(1): " SIGNIFICAND_FMT ", " SIGNIFICAND_FMT "\n", __func__, a, b);

      uint8_t carry;
      if((retcode = bcd_significand_add(a, b, &op1->significand, &carry)) != true) { break; }
      DBG_PRINT("%s(2): " SIGNIFICAND_FMT "\n", __func__, op1->significand);

      /* If exactly one of the operands is negative:
       * - If we have carry, then the result is positive.
       * - If we have no carry, then the result is negative.
       */
      if( ((op1->sign ==  true) && (op2->sign == false)) ||
          ((op1->sign == false) && (op2->sign ==  true)) )
      {
        if(carry == 0)
        {
          if((retcode = bcd_tens_complement(op1->significand, &op1->significand)) == false) break;
        }
        op1->sign = (carry == 0) ? true : false;
      }

      /* Otherwise the signs are the same.  The sum will have the same sign. */
      else
      {
        /* If the result is negative, do 10s complement now. */
        if(op1->sign == true)
        {
          if((retcode = bcd_tens_complement(op1->significand, &op1->significand)) == false) break;
          carry ^= 1;
        }

        /* If carry, shift significand, insert carry, bump exponent. */
        if(carry != 0)
        {
          significand_t carry_big = carry;
          op1->significand >>= 4;
          op1->significand |= (carry_big << 60);
          op1->exponent++;
        }
      }
      DBG_PRINT("%s(3): " SIGNIFICAND_FMT " %d %d.\n", __func__, op1->significand, op1->exponent, op1->sign);

      /* Clear out any leading zeroes in the significand. */
      while( (op1->significand != 0) && ((op1->significand & 0xF000000000000000ll) == 0) )
      {
        op1->significand <<= 4;
        op1->exponent--;
      }
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
      DBG_PRINT("%s(): " SIGNIFICAND_FMT " - " SIGNIFICAND_FMT "\n", __func__, op1->significand, op2->significand);
printf("%s(): " SIGNIFICAND_FMT " - " SIGNIFICAND_FMT "\n", __func__, op1->significand, op2->significand);

      significand_t o2;
      if((retcode = bcd_tens_complement(op2->significand, &o2)) != true) break;
      DBG_PRINT("%s(): o2 = " SIGNIFICAND_FMT "\n", __func__, o2);
printf("%s(): o2 = " SIGNIFICAND_FMT "\n", __func__, o2);

      uint8_t carry;
      if((retcode = bcd_significand_add(op1->significand, o2, &op1->significand, &carry)) != true) break;
      DBG_PRINT("%s(): RESULT " SIGNIFICAND_FMT " CARRY %d.\n", __func__, op1->significand, carry);
printf("%s(): RESULT " SIGNIFICAND_FMT " CARRY %d.\n", __func__, op1->significand, carry);

      /* If carry, then the result is positive.
       * If no carry, then the result is negative. */
      op1->sign = (carry != 0) ? false : true;
      if(op1->sign == true)
      {
        if((retcode = bcd_tens_complement(op1->significand, &op1->significand)) != true) break;
        DBG_PRINT("%s(): NEGATIVE " SIGNIFICAND_FMT ".\n", __func__, op1->significand);
printf("%s(): NEGATIVE " SIGNIFICAND_FMT ".\n", __func__, op1->significand);
      }

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
        if(--buf_size == 0) { break; } else { buf[buf_x++] = 'e'; }

        int16_t exponent = this->exponent;
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

#if 0
static uint32_t
add(uint32_t a, uint32_t b)
{
  uint32_t t1 = a + 0x06666666;
  uint32_t t2 = t1 + b;
  uint32_t t3 = t1 ^ b;
  uint32_t t4 = t2 ^ t3;
  uint32_t t5 = ~t4 & 0x11111110;
  uint32_t t6 = (t5 >> 2) | (t5 >> 3);
  return t2 - t6;
}

static uint32_t
tencomp(uint32_t a)
{
  uint32_t t1 = 0xF9999999 - a;
  uint32_t t2 = t1 + 0x06666666;
  uint32_t t3 = t2 + 0x00000001;
  uint32_t t4 = t2 ^ 0x00000001;
  uint32_t t5 = t3 ^ t4;
  uint32_t t6 = ~t5 & 0x11111110;
  uint32_t t7 = (t6 >> 2) | (t6 >> 3);
  return t3 - t7;
}
#endif

bool
bcd_test(void)
{
  bool retcode = false;

#if 0
uint32_t a = 0x12345678;
uint32_t b = 0x87654321;

uint32_t c = add(a, b);
printf("%08X + %08X = %08X\n", a, b, c);

uint32_t d = 0x12345678;
uint32_t e = tencomp(d);
printf("%08X -> %08X\n", d, e);

uint32_t f = 0x00006789;
uint32_t g = 0x00051234;
         g = tencomp(g);
uint32_t h = add(f, g);
printf("%08X + %08X = %08X\n", f, g, h);

  significand_t s1, s2;
  s1  = 0x1234567812345678ll;
  s1  = 0x9876543210987654ll;
  retcode = bcd_tens_complement(s1, &s2);
  printf("s1 = 0x%016llX: s2 = 0x%016llX.\n", s1, s2);

return true;
#endif

  /* Loop through some decimal numbers.  This tests the basic functionality of
   * the bcd class.  We're checking to make sure it can handle any type of
   * decimal number that we might throw at it. */
  typedef struct bcd_test {
    const char  *name;
    const char  *src;
    const char  *dst;
  } bcd_test;
  bcd_test tests[] = {
#if 0
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
#endif
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
#if 0
    { "BCD_ADD_01",                "1"                 ,                "2"                 , bcd_op_add,                "3"                    }, // Debug
    { "BCD_ADD_02",         "99999999"                 ,                "1"                 , bcd_op_add,        "100000000"                    }, // 33-bits
    { "BCD_ADD_03",        "999999999"                 ,                "1"                 , bcd_op_add,       "1000000000"                    }, // Carry.
    { "BCD_ADD_04", "1234567890123456"                 , "9876543210987654"                 , bcd_op_add,                "1.111111110111111e16" }, // 17 digits
    { "BCD_ADD_05",                 ".1234567890123456", "9876543210987654"                 , bcd_op_add, "9876543210987654"                    }, // 16.0 + 0.16 digits.
    { "BCD_ADD_06",             "1234s"                ,             "4321"                 , bcd_op_add,             "3087"                    }, // 1st num neg.
    { "BCD_ADD_07",             "8766"                 ,             "4321"                 , bcd_op_add,            "13087"                    }, // Like previous, but pos.
    { "BCD_ADD_08",              "123"                 ,             "1234"                 , bcd_op_add,             "1357"                    }, // Pos + Pos.
    { "BCD_ADD_09",              "456s"                ,              "123"                 , bcd_op_add,             "-333"                    }, // Neg + Pos = Neg.
    { "BCD_ADD_10",              "456s"                ,             "1234"                 , bcd_op_add,              "778"                    }, // Neg + Pos = Pos.
    { "BCD_ADD_11",              "789"                 ,             "1234s"                , bcd_op_add,             "-445"                    }, // Pos + Neg = Neg.
    { "BCD_ADD_12",              "789"                 ,              "123s"                , bcd_op_add,              "666"                    }, // Pos + Neg = Pos.
    { "BCD_ADD_13",              "202s"                ,             "1234s"                , bcd_op_add,            "-1436"                    }, // Neg + Neg.
    { "BCD_ADD_14",             "9990s"                ,             "1234s"                , bcd_op_add,           "-11224"                    }, // Neg with carry.
    { "BCD_ADD_15",               "10.5"               ,                 ".5"               , bcd_op_add,               "11"                    }, // decimal to whole.
    { "BCD_ADD_16",                 ".1111111111111111",                 ".1111111111111111", bcd_op_add,                "0.222222222222222"    }, // No carry, truncate.
    { "BCD_ADD_17",             "1000"                 ,             "1000"                 , bcd_op_add,             "2000"                    }, // Trailing zeroes.
    { "BCD_ADD_18",                 ".00001"           ,                 ".00001"           , bcd_op_add,                "0.00002"              }, // Significant zeroes.
#endif

    { "BCD_SUB_01",                "5"                 ,                "2"                 , bcd_op_sub,                "3"                    }, // Debug.
    { "BCD_SUB_02",                "0"                 ,                "1"                 , bcd_op_sub,               "-1"                    }, // Neg num.
    { "BCD_SUB_03",            "12345"                 ,             "1234"                 , bcd_op_sub,            "11111"                    }, // Pos - Pos = Pos.
    { "BCD_SUB_04",            "54321"                 ,            "91234"                 , bcd_op_sub,           "-36913"                    }, // Pos - Pos = Neg.
    { "BCD_SUB_05",            "12345"                 ,              "123.4s"              , bcd_op_sub,            "12468.4"                  }, // Pos - Neg = Pos.
    { "BCD_SUB_06",              "432.1s"              ,                "7.5678"            , bcd_op_sub,             "-439.6678"               }, // Neg - Pos = Neg.
    { "BCD_SUB_07",             "1225s"                ,               "34.95s"             , bcd_op_sub,            "-1190.05"                 }, // Neg - Neg = Neg.
    { "BCD_SUB_08", "1111111111111111s"                , "1234567890123456s"                , bcd_op_sub,  "123456779012345"                    }, // Neg - Neg = Pos.

    { "BCD_MUL_01",                "1"                 ,                "0"                 , bcd_op_mul,                "0"                    }, // Mult = 0.

    { "BCD_DIV_01",            "24680"                 ,                "2"                 , bcd_op_div,            "12340"                    }, // Simple div.
  };
  size_t bcd_math_test_size = (sizeof(math_tests) / sizeof(bcd_math_test));

  for(x = 0; x < bcd_math_test_size; x++)
  {
    bcd_math_test *t = &math_tests[x];
    const char *val1 = t->val1;
    const char *val2 = t->val2;
    printf("  %s\n", t->name);

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

    DBG_PRINT("strcmp(%s, %s)\n", t->result, buf);
    if((retcode = (strcmp(t->result, buf) == 0)) != true)            return false;

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

