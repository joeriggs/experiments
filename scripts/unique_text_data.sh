#!/bin/sh

###############################################################################
# This script will create a text file that contains unique data all the way
# through.  This allows the user to scan things like WireShark or mem buffers
# and figure out what data they are looking at.
#
# WARNING:  This script will definitely run in a CygWin shell, but it's a
#           little slow.  It runs much faster if run from a CentOS shell.
###############################################################################

# Modify the following variables via the cmdline.
NUM_BYTES_PER_FILE=16
NUM_FILES=8

DO_VERBOSE=1

#########
usage() {
  printf "Create a text file that contains unique data throughout the file.\n"
  printf "  Usage: unique_text_data.sh [-f <files> ] [-s <size>] [-s]\n"
  printf "\n"
  printf "    -f = The number of files to create.\n"
  printf "         <files> = Number of files.  Default is %d.\n" ${NUM_FILES}
  printf "\n"
  printf "    -s = The size of the file.\n"
  printf "         <size>  = Number of bytes per file.  Default is %d.\n" ${NUM_BYTES_PER_FILE}
  printf "\n"
  printf "    -q = Quiet mode.  No console output.\n"
  printf "\n"
  exit 1
}

##############################################################################
##############################################################################
########################### Start processing here ############################
##############################################################################
##############################################################################

#########
# Parse the command-line.  First, break it down by option.
while getopts "f:s:hq" opt
do
  case "$opt" in

  "h") usage
       ;;

  "f") NUM_FILES=${OPTARG}
       ;;

  "s") NUM_BYTES_PER_FILE=${OPTARG}
       ;;

  "q") DO_VERBOSE=0
       ;;

   * ) echo "Invalid cmdline switch \"$opt\"."
       usage
       ;;

  esac
done

if [ ${DO_VERBOSE} = 1 ]; then
  (( TOTAL_BYTES = NUM_FILES * NUM_BYTES_PER_FILE ))
  printf "\n"
  printf "Creating the following:\n"
  printf "\n"
  printf "%12d files\n"          ${NUM_FILES}
  printf "%12d bytes/file\n" ${NUM_BYTES_PER_FILE}
  printf "_______________________\n"
  printf "%12d total bytes\n"    ${TOTAL_BYTES}
  printf "\n"
fi

# Internal counters.
FILE_NUM=0
RECORD_NUMBER=0
NAME="./test.txt"

while [ ${FILE_NUM} != ${NUM_FILES} ]; do

  # Size of the string to copy to each file.  This will be adjusted for the
  # last record so that the length is correct.
  (( FORMAT_SIZE = 64 ))
  (( RECORD_SIZE = FORMAT_SIZE - 1 ))

  # The name of the current file.
  NAME=`printf "file.%04d" ${FILE_NUM}`
  rm -f ${NAME}

  # The total number of bytes written to the current file.
  TOTAL_WRITTEN=0

  while [ ${TOTAL_WRITTEN} -lt ${NUM_BYTES_PER_FILE} ]; do

    (( POST_WRITE_SIZE = TOTAL_WRITTEN + FORMAT_SIZE ))

    if [ ${POST_WRITE_SIZE} -gt ${NUM_BYTES_PER_FILE} ]; then
      (( FORMAT_SIZE = NUM_BYTES_PER_FILE - TOTAL_WRITTEN ))
      (( RECORD_SIZE = FORMAT_SIZE - 1 ))
    fi

    printf "%0${RECORD_SIZE}d\n" ${RECORD_NUMBER} >>  ${NAME}

    (( RECORD_NUMBER = RECORD_NUMBER + 1 ))

    (( TOTAL_WRITTEN = TOTAL_WRITTEN + FORMAT_SIZE ))

    (( PCT_DONE = TOTAL_WRITTEN * 100 / NUM_BYTES_PER_FILE ))

    printf "File %s: Rec %015d: Bytes %015d: %3d%% Done.     \r" ${NAME} ${RECORD_NUMBER} ${TOTAL_WRITTEN} ${PCT_DONE}

  done

  printf "\n"

  (( FILE_NUM = FILE_NUM + 1 ))

done

printf "\n\n"

