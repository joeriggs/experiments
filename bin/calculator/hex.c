/* This is a Hexadecimal implementation.  We need to be able to do hex math
 * so this ADT provides that capability.
 *
 * The hex ADT doesn't support floating point math.  All hex numbers are
 * treated as 64-bit integers.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "hex.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the hex class. */
struct hex {

  /* This is the hex number. */
  uint64_t val;
};
  
/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

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
hex_op_add(hex *op1,
           hex *op2)
{
  bool retcode = false;

  if((op1 != (hex *) 0) && (op2 != (hex *) 0))
  {
    op1->val += op2->val;
    retcode = true;
  }
    
  return retcode;
}

/* This is the BCD subtraction function.
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
hex_op_sub(hex *op1,
           hex *op2)
{
  bool retcode = false;

  if((op1 != (hex *) 0) && (op2 != (hex *) 0))
  {
    op1->val -= op2->val;
    retcode = true;
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
hex_op_mul(hex *op1,
           hex *op2)
{
  bool retcode = false;

  if((op1 != (hex *) 0) && (op2 != (hex *) 0))
  {
    op1->val *= op2->val;
    retcode = true;
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
hex_op_div(hex *op1,
           hex *op2)
{
  bool retcode = false;

  /* Don't allow divide by zero. */
  if((op1 != (hex *) 0) && (op2 != (hex *) 0) && (op2->val != 0))
  {
    op1->val /= op2->val;
    retcode = true;
  }
    
  return retcode;
}

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Create a new hex object.  This object can be used to access the hex class.
 *
 * Input:
 *   N/A.
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.
 */
hex *
hex_new(void)
{
  hex *this = malloc(sizeof(*this));

  if(this != (hex *) 0)
  {
    /* Start with zero. */
    this->val  = 0;
  }

  return this;
}

/* Delete a hex object that was created by hex_new().
 *
 * Input:
 *   this = A pointer to the hex object.
 *
 * Output:
 *   true  = success.  this is deleted.
 *   false = failure.  this is undefined.
 */
bool
hex_delete(hex *this)
{
  bool retcode = false;

  if(this != (hex *) 0)
  {
    free(this);
    retcode = true;
  }

  return retcode;
}

/* Attempt to add a character to the hex object.  The character is checked to
 * see if it's a valid part of a hexadecimal number.  If it's valid, it's added.
 * Note that it's possible for a character to be dropped because we don't have
 * room for it.
 *
 * We only allow the user to insert 16 digits.  After that, we drop them.
 *
 * Input:
 *   this = A pointer to the hex object.
 *
 *   c    = The char to add.  If it's valid, we use it.  If it's not valid,
 *          then we return false.
 *
 * Output:
 *   true  = success.  c is valid, and it has been added to this (if it fits).
 *   false = failure.  c is NOT a number OR we were unable to add c to this.
 */
bool
hex_add_char(hex *this,
             char c)
{
  bool retcode = false;

  /* An 'S' toggles the +/- sign. */
  if((c & 0xDF) == 'S')
  {
    this->val = 0 - this->val;
    retcode = true;
  }

  /* Check to see if it's a hex a digit. */
  else
  {
    if((c >= '0') && (c <= '9'))
    {
      c -= 0x30;
      retcode = true;
    }
    else 
    {
      c &= 0xDF;
      c -= 0x37;
      if((c >= 0x0A) && (c <= 0x0F))
      {
        retcode = true;
      }
    }

    if(retcode == true)
    {
      /* If the value is already full, then silently drop the character. */
      if((this->val & 0x7000000000000000ll) == 0)
      {
        this->val <<= 4;
        this->val |= c;
      }
    }
  }

  return retcode;
}

/* Create an ASCII string the represents the current value of the number.
 *
 * Input:
 *   this     = A pointer to the hex object.
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
hex_to_str(hex  *this,
           char  *buf,
           size_t buf_size)
{
  bool retcode = false;

  if( (this != (hex *) 0) && (buf != (char *) 0) && (buf_size > 0) )
  {
    int buf_x = 0;

    do
    {
      int64_t val = this->val;

      DBG_PRINT("%s(): 0x%016llX.\n", __func__, val);

      /* If it's zero, then it's zero. */
      if(val == 0ll)
      {
        if(--buf_size == 0) { break; } else { buf[buf_x++] = '0'; }
        break;
      }

      /* Skip insignificant zeroes. */
      int i;
      for(i = 16; (i > 0) && (((val >> ((i - 1) * 4)) & 0xF) == 0); i--);

      /* Now add the actual digits. */
      for( ; i > 0; i--)
      {
        int shift = (i - 1) * 4;
        char c = (val >> shift) & 0xF;

        /* Convert to ASCII. */
        c += (c <= 9) ? 0x30 : 0x37;

        /* Insert the digit. */
        if(--buf_size == 0) { break; } else { buf[buf_x++] = c; }
      }
    } while(0);

    buf[buf_x] = 0;
    retcode = true;
  }

  return retcode;
}
/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#ifdef TEST

bool
hex_test(void)
{
  bool retcode = false;

  /* Loop through some hex numbers.  This tests the basic functionality of
   * the hex class.  We're checking to make sure it can handle any type of
   * hex number that we might throw at it. */
  typedef struct hex_test {
    const char  *src;
    const char  *dst;
  } hex_test;
  hex_test tests[] = {
    {                 "1"  ,                "1" }, // A single-digit number.
    {               "123"  ,              "123" }, // Simple integer value.
    {            "123000"  ,           "123000" }, // Integer with trailing zeroes.
    {         "000123000"  ,           "123000" }, // Insignificant leading zeroes.
    {  "FEDCBA9876543210"  , "FEDCBA9876543210" }, // Full size (including MSB).
    {  "FEDCBA9876543210S" ,  "123456789ABCDF0" }, // Negative number.
    { "123456789abcdef01"  , "123456789ABCDEF0" }, // Too big (truncate).
  };
  size_t hex_test_size = (sizeof(tests) / sizeof(hex_test));

  int x;
  for(x = 0; x < hex_test_size; x++)
  {
    hex_test *t = &tests[x];
    printf("  %s\n", t->src);

    hex *this = hex_new();
    if((retcode = (this != (hex *) 0)) != true)                    return false;

    const char *src = t->src;
    while(*src)
    {
      if((retcode = hex_add_char(this, *src++)) != true)           return false;
    }

    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if((retcode = hex_to_str(this, buf, sizeof(buf))) != true)     return false;

    DBG_PRINT("strcmp(%s, %s)\n", t->dst, buf);
    if((retcode = (strcmp(t->dst, buf) == 0)) != true)             return false;
  }
  
  /* Math operations. */
  typedef struct hex_math_test {
    const char  *name;
    const char  *val1;
    const char  *val2;
    bool (*func)(hex *val1, hex *val2);
    const char  *result;
  } hex_math_test;
  hex_math_test math_tests[] = {
    { "HEX_ADD_01",                "1",     "2", hex_op_add,                "3" },
    { "HEX_ADD_02", "FFFFFFFFFFFFFFFF",     "1", hex_op_add,                "0" },
    { "HEX_SUB_01",                "0",     "1", hex_op_sub, "FFFFFFFFFFFFFFFF" },
    { "HEX_SUB_02",             "1000",   "123", hex_op_sub,              "EDD" },
    { "HEX_MUL_01",                "1",     "0", hex_op_mul,                "0" },
    { "HEX_MUL_02",             "1234",  "5678", hex_op_mul,          "6260060" },
    { "HEX_MUL_03", "1000000000000000",    "10", hex_op_mul,                "0" },
    { "HEX_DIV_01",        "136bdbca4",  "1234", hex_op_div,           "11040D" },
    { "HEX_DIV_02",            "fffff", "fffff", hex_op_div,                "1" },
  };
  size_t hex_math_test_size = (sizeof(math_tests) / sizeof(hex_math_test));

  for(x = 0; x < hex_math_test_size; x++)
  {
    hex_math_test *t = &math_tests[x];
    const char *val1 = t->val1;
    const char *val2 = t->val2;
    printf("  %s\n", t->name);

    hex *obj1 = hex_new();
    if((retcode = (obj1 != (hex *) 0)) != true)                    return false;
    while(*val1)
    {
      if((retcode = hex_add_char(obj1, *val1++)) != true)          return false;
    }

    hex *obj2 = hex_new();
    if((retcode = (obj2 != (hex *) 0)) != true)                    return false;
    while(*val2)
    {
      if((retcode = hex_add_char(obj2, *val2++)) != true)          return false;
    }

    if((retcode = t->func(obj1, obj2)) != true)                    return false;
    
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if((retcode = hex_to_str(obj1, buf, sizeof(buf))) != true)     return false;

    DBG_PRINT("strcmp(%s, %s)\n", t->result, buf);
    if((retcode = (strcmp(t->result, buf) == 0)) != true)          return false;

    if((retcode = hex_delete(obj1)) != true)                       return false;
    if((retcode = hex_delete(obj2)) != true)                       return false;
  }

  /* Divide by zero test.  We're not fiddling around with creating a bunch of
   * nifty little objects.  We're building them and firing off the test. */
  printf("Divide by zero test.\n");
  hex *o1 = hex_new(); o1->val = 0x1;
  hex *o2 = hex_new(); o1->val = 0x0;
  if(hex_op_div(o1, o2) != false)                                  return false;
  hex_delete(o1); hex_delete(o2);

  return retcode;
}

#endif // TEST

