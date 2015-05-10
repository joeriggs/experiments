#!/bin/sh

###############################################################################
# This script will create a bunch of unique files and distribute them among
# one or more directories.  The files will all be the exact same size, but they
# will all contain unique contents.
#
# WARNING:  This script will definitely run in a CygWin shell, but it's a
#           little slow.  It runs much faster if run from a CentOS shell.
###############################################################################

##############################################################################
##############################################################################
#### Modify the following 3 variables to specify what you want to create. ####
#### The variables will allow you to specify the number of files, number  ####
#### of directories, and size of each file.                               ####
####                                                                      ####
#### NUM_FILES is the total number of files to create.                    ####
####                                                                      ####
#### NUM_DIRS  is the number of dirs to create.  The files will be evenly ####
####           distributed over those dirs.                               ####
####                                                                      ####
#### NUM_BYTES_PER_FILE is obviously the size of each file.               ####
####                                                                      ####
##############################################################################
##############################################################################

# This is the number of files to create.  You can create as many as you want.
# There isn't a restriction.  But for the casual user, some interesting choices
# would be:
#    1024 =   1K files
#    2048 =   2K files
#    4096 =   4K files
#    8192 =   8K files
#   16384 =  16K files
#   32768 =  32K files
#   65536 =  64K files
#  131072 = 128K files
#  262144 = 256K files
#  524288 = 512K files
# 1048576 =   1M files
# 2097152 =   2M files
# 4194304 =   4M files
# 8388608 =   8M files
NUM_FILES=256

# This is the number of directories to create.  The files will be divided
# evenly among the directories (as closely as possible).
NUM_DIRS=16

# This is the size of each file.  I wouldn't push my luck too far with this
# value.  I've tried it with sizes as high as 1,000,000, but there is probably
# an upper limit somewhere.  It's used to create the format for a printf call,
# so the ultimate limit is whatever printf can handle.  And besides, this tool
# wasn't intended to create huge files.  It was intended to create thousands or
# millions of small files.
#
# A particularly useful value is 16.  Our Crypto Blocks are 16-bytes, so if you
# specify a size of 16 you will create a whole bunch of files in which each
# config-block-sized chunk is completely unique.
NUM_BYTES_PER_FILE=16

DO_VERBOSE=1

# This is the template for the file names that we create.
FILE_TEMPLATE=urandom%06d.dat

#########
usage() {
  printf "Create a bunch of unique files.  Each file contains unique data.\n"
  printf "  Usage: bunch_o_files.sh [-f <files> ] [-d <dirs>] [-s <size>] [-s]\n"
  printf "\n"
  printf "    -f = Number of files to create.\n"
  printf "         <files> = Number of files to create.  Default is %d.\n" ${NUM_FILES}
  printf "\n"
  printf "    -d = Number of directories to create.  The files will be evenly divided\n"
  printf "         over the directories.\n"
  printf "         <dirs>  = Number of directories to create.  Default is %d.\n" ${NUM_DIRS}
  printf "\n"
  printf "    -s = The size of each file.  All files are the same size.\n"
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
while getopts "f:d:s:hq" opt
do
  case "$opt" in

  "h") usage
       ;;

  "f") NUM_FILES=${OPTARG}
       ;;

  "d") NUM_DIRS=${OPTARG}
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

# This is the number of files to place in each directory.
(( NUM_FILES_PER_DIR = NUM_FILES / NUM_DIRS ))

# This is the total number of bytes.
(( NUM_BYTES = NUM_FILES * NUM_BYTES_PER_FILE ))

if [ ${DO_VERBOSE} = 1 ]; then
  printf "\n"
  printf "Creating the following:\n"
  printf "\n"
  printf "%12d dirs\n"           ${NUM_DIRS}
  printf "%12d files per dir\n"  ${NUM_FILES_PER_DIR}
  printf "____________\n"
  printf "%12d total files\n"    ${NUM_FILES}
  printf "%12d bytes per file\n" ${NUM_BYTES_PER_FILE}
  printf "____________\n"
  printf "%12d total bytes\n"    ${NUM_BYTES}
  printf "\n"
fi

# Size of the string to copy to each file.
(( FORMAT_SIZE = NUM_BYTES_PER_FILE - 1 ))

# Internal counters.
NEXT_DIR_NUM=1
(( LAST_DIR_NUM = NEXT_DIR_NUM + NUM_DIRS ))

NEXT_FILE_NUM=1
LAST_FILE_NUM=1

while [ ${NEXT_DIR_NUM} != ${LAST_DIR_NUM} ]; do

  # If we're using more than 1 subdirectory, create the next one now.  If all
  # of the files go into 1 directory, there is no need to create a dir.
  if [ ${NUM_DIRS} -gt 1 ]; then
    DIR_NAME=`printf "%04d" ${NEXT_DIR_NUM}`
    mkdir ${DIR_NAME} > /dev/null 2>&1
    [ "$?" != "0" ] && printf "\nError creating directory %s\n" ${DIR_NAME}
  else
    DIR_NAME="."
  fi

  (( LAST_FILE_NUM = LAST_FILE_NUM + NUM_FILES_PER_DIR ))

  while [ ${NEXT_FILE_NUM} != ${LAST_FILE_NUM} ]; do

    NAME=`printf "%s/${FILE_TEMPLATE}" ${DIR_NAME} ${NEXT_FILE_NUM}`

    if [ ${DO_VERBOSE} = 1 ]; then
      printf "%s\r" ${NAME}
    fi

    printf "%0${FORMAT_SIZE}d\n" ${NEXT_FILE_NUM} >  ${NAME}

    (( NEXT_FILE_NUM = NEXT_FILE_NUM + 1 ))

  done

  (( NEXT_DIR_NUM = NEXT_DIR_NUM + 1 ))

done

