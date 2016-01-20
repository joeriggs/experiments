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
  int mknod_rc = mknod(PIPE_NAME, S_IFIFO, 0);

  if((mknod_rc == 0) || ((mknod_rc == -1) && (errno == EEXIST))) {
    return true;
  }
  else {
    return false;
  }
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
  int fd = open(PIPE_NAME, open_flag);
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
