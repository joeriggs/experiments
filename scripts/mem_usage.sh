#!/bin/sh

# This script will look through /proc/[pid]/maps for a process and count the
# number of bytes of memory currently allocated.
#
# Specify the name of the process on the cmdline.

PROCESS="${1}"

PID=$(set -- $(ps -ef | grep ${PROCESS} | head -1); echo $2)
MAPS="/proc/${PID}/maps"
echo "Reading ${MAPS}: PID ${PID}"

sed -e "s/ .*//g;" ${MAPS} > /tmp/maps.txt

TOT_SIZE=0
TOT_GAP=0

END_UPPER=0

FOUND_KERNEL=0

printf "Begin       End        Bytes             Total     Gap Before    Total Gap\n"
for rec in `cat /tmp/maps.txt`; do

  BEG=$(set -- $(echo $rec | sed -e"s/-/ /"); echo $1)
  BEG_UPPER=`echo ${BEG} | awk '{print toupper($0)}'`

  GAP=`echo "ibase=16;${BEG_UPPER}-${END_UPPER}" | bc`
  TOT_GAP=`expr ${TOT_GAP} + ${GAP}`

  END=$(set -- $(echo $rec | sed -e"s/-/ /"); echo $2)
  END_UPPER=`echo ${END} | awk '{print toupper($0)}'`

  LEN_UPPER=`echo "ibase=16;${END_UPPER}-${BEG_UPPER}" | bc`
  TOT_SIZE=`echo "${TOT_SIZE}+${LEN_UPPER}" | bc`

  if [ ${FOUND_KERNEL} = 0 ] && [ "${BEG_UPPER}" = "C0000000" ]; then
    printf "----- End of User Space ----- Begin Kernel Space -----\n"
    FOUND_KERNEL=1
  fi

  printf "%s -> %s = 0x%08x %12d : %12d %12d\n" ${BEG_UPPER} ${END_UPPER} ${LEN_UPPER} ${TOT_SIZE} ${GAP} ${TOT_GAP}

  if [ ${FOUND_KERNEL} = 0 ] && [ "${END_UPPER}" = "C0000000" ]; then
    printf "----- End of User Space ----- Begin Kernel Space -----\n"
    FOUND_KERNEL=1
  fi

done

