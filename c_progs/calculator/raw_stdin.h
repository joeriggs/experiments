/* A simple class that configures the console device as a raw device so the
 * program can read keyboard input one character at a time as it is being
 * entered by the user.
 */
#ifndef __RAW_STDIN_H__
#define __RAW_STDIN_H__

/****************************** CLASS DEFINITION ******************************/

typedef struct raw_stdin raw_stdin;

/********************************* PUBLIC API *********************************/

raw_stdin *raw_stdin_new(void);

bool raw_stdin_delete(raw_stdin *this);

bool raw_stdin_getchar(raw_stdin *this, char *c);

/********************************** TEST API **********************************/

#if defined(TEST)

bool raw_stdin_test(void);

#endif // TEST

#endif // __RAW_STDIN_H__

