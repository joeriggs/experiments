/* A simple class that configures the console device as a raw device so the
 * program can read keyboard input one character at a time as it is being
 * entered by the user.
 */

typedef struct raw_stdin raw_stdin;

extern raw_stdin *raw_stdin_new(void);
extern const char raw_stdin_getchar(raw_stdin *this);
extern void raw_stdin_free(raw_stdin *this);

