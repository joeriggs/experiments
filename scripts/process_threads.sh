#!/bin/sh

###############################################################################
# Give the name of program and it'll tell you where the various threads are
# running.  This is useful for matching thread IDs against the thread numbers
# that are used by gdb.
#
# NOTE: If there is more than one instance of the process, it will only report
# the threads for the first instance it finds.  You can easily extend the
# script to look for more than one instance of the process.
###############################################################################

PROCESS_NAME="${1}"
[ -z "${PROCESS_NAME}" ] && echo "Please specify the name of a process" && exit 1

PID=$( set -- $(ps -A | grep ${PROCESS_NAME}); echo $1)

echo "${PROCESS_NAME} PID = ${PID}"

for THREAD in `ls /proc/${PID}/task`; do
  CPU=$( set -- $(cat /proc/${PID}/task/${THREAD}/stat); echo ${39})
  echo "THREAD ${THREAD} is on CPU ${CPU}"
done
  

