#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>

static const char *source = "/opt/PftestFUSE";
static const char *target = "/opt/Pftest";

int main(int argc, char **argv)
{
	int retcode;
	char *errStr;
	int myErrno;
	int createdSourceDir = 0;

	struct stat s;
	retcode = stat(source, &s);
	myErrno = errno;
	errStr = strerror(myErrno);
	printf("stat() returned %d.  error %s.  st_mode = %o.\n", retcode, (retcode == 0) ? "" : errStr, s.st_mode);
	if((retcode == -1) && (myErrno = ENOENT)) {
		retcode = mkdir(source, 0777);
		myErrno = errno;
		errStr = strerror(myErrno);
		printf("mkdir() returned %d.  error %s.\n", retcode, (retcode == 0) ? "" : errStr);

		if(retcode == 0) {
			createdSourceDir = 1;
		}
	}

	if(retcode == 0) {

		retcode = mount(source, target, NULL, MS_BIND, 0);
		myErrno = errno;
		errStr = strerror(myErrno);
		printf("mount() returned %d.  error %s.\n", retcode, (retcode == 0) ? "" : errStr);

		retcode = umount(target);
		myErrno = errno;
		errStr = strerror(myErrno);
		printf("umount() returned %d.  error %s.\n", retcode, (retcode == 0) ? "" : errStr);

	}

	if(createdSourceDir) {
		retcode = rmdir(source);
		myErrno = errno;
		errStr = strerror(myErrno);
		printf("rmdir() returned %d.  error %s.\n", retcode, (retcode == 0) ? "" : errStr);
	}

	return 0;
}

