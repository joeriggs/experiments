/*******************************************************************************
 * This file contains macros and functions that are used by the client and
 * server.
 ******************************************************************************/

#pragma once

#define PIPE_NAME "./test_pipe"

/* Create the device node that will server as the anchor point for the named
 * pipe.  Both ends (client and server) can try to create it. */
static bool
make_pipe_node()
{
  int rc = false;

#if 0
  /* Use this if you want to experiment with mknod(). */
  int mknod_rc = mknod(PIPE_NAME, S_IFIFO, 0);
  printf("%s(): mknod(%s, %x, 0) returned = %d.\n", __func__, PIPE_NAME, S_IFIFO, mknod_rc);
#else
  /* Use this if you want to experiment with mkfifo(). */
  int mknod_rc = mkfifo(PIPE_NAME, 0777);
  printf("%s(): mkfifo(%s, 0777) returned = %d.\n", __func__, PIPE_NAME, mknod_rc);
#endif

  if((mknod_rc == 0) || ((mknod_rc == -1) && (errno == EEXIST))) {

    /* Make it accessible by any user. */
    int rc = chmod(PIPE_NAME, 0777);
    printf("%s(): chmod() returned %d.\n", __func__, rc);

    if(rc == 0) {
      return true;
    }
  }

  return rc;
}

/* Create a pipe.  The caller specifies whether we're the writer or the
 * reader.
 *
 * On success, returns the handle to the pipe.
 * On failure, returns -1.
 */
static int
create_pipe(bool is_server)
{
  int open_flag = (is_server) ? O_WRONLY : O_RDONLY;
  int fd = open(PIPE_NAME, open_flag | O_NONBLOCK);
  printf("%s(): open(%s, %x) returned = %d.\n", __func__, PIPE_NAME, open_flag, fd);
  if(fd != -1) {

    /* Make sure the pipe is non-blocking. */
    fcntl(fd, F_SETFL, (fcntl(fd, F_GETFL) | O_NONBLOCK));
  }

  return fd;
}

/* Write a mesage into the pipe.
 *
 * On success, returns the number of bytes writen.
 * On failure, returns -1.
 */
static int
write_to_pipe(int fd, const char *data, size_t len)
{
  return write(fd, data, len);
}

/* Read a message from the pipe.
 *
 * On success, returns the number of bytes read.
 * On failure, returns -1.
 */
static int
read_from_pipe(int fd, char *data, size_t max_len)
{
  int bytes_read = read(fd, data, max_len);
  if(bytes_read == -1) {
    bytes_read = (errno == EAGAIN) ? 0 : -1;
  }

  return bytes_read;
}
