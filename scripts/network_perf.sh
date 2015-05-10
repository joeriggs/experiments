#!/bin/sh

###############################################################################
# This shell script will print network throughput stats for a Linux computer.
#
# It can print either of the following:
# 1. Ethernet throughput.  This is done by combining the "RX bytes" and
#    "TX bytes" from the ifconfig command, and then calculating the mbps.
# 2. Displaying the number of NFS Calls per second using statistics from the
#    nfsstat command.
#
# Written by Joe Riggs.  Please let me know if you have a better way to do
# this.
#
###############################################################################

###############################################################################
# Select one of the following.  It will control which statistics are shown.
DO_THROUGHPUT=1
   THROUGHPUT_AVG=60
   THROUGHPUT_MBPS=1
   THROUGHPUT_BYTES=0
DO_NFS_CALLS=0

###############################################################################
# This function will use statistics from the ifconfig command to calculate the
# throughput on the Ethernet interface.
###############################################################################
throughput_stats() {
  local PREV_BYTES=0

  local LOOP_COUNT=0
  local BYTES_TOTAL=0

  local LRU_INDEX=0
  while [ ${LRU_INDEX} -lt ${THROUGHPUT_AVG} ]; do
    local BYTES_ARRAY[${LRU_INDEX}]=0
    local TIMES_ARRAY[${LRU_INDEX}]=0
    LRU_INDEX=`expr ${LRU_INDEX} + 1`
  done
  LRU_INDEX=0

  local LAST_TIME=`date +%s.%N`
  while [ 1 ]; do

    local TS=`date`
    LOOP_COUNT=`expr ${LOOP_COUNT} + 1`

    # Get the current bytes counts from ifconfig.  Try eth1, then try eth0.
    local LOG_TMP=/tmp/ifconfig.output
    ifconfig eth0 > ${LOG_TMP} 2>&1
    grep -q "Device not found" ${LOG_TMP}
    [ "$?" = 0 ] && ifconfig eth1 > ${LOG_TMP} 2>&1
    grep -q "Device not found" ${LOG_TMP}
    [ "$?" = 0 ] && echo "Can't find your Ethernet device." && exit 0

    # Grab a timestamp right away.  Want to grab the byte counts and timestamp
    # as close together as possible.  Then calculate the elapsed time.
    local CURR_TIME=`date +%s.%N`
    local ELAPSED_TIME_SEC=$(echo "scale=1; ${CURR_TIME} - ${LAST_TIME}" | bc -l)
    LAST_TIME=${CURR_TIME}

    cat ${LOG_TMP} | grep "RX bytes" | sed -e "s/bytes://g;" > /tmp/ifconfig 2>&1
    local RX_BYTES=$(set -- $(cat /tmp/ifconfig); echo $2)
    local TX_BYTES=$(set -- $(cat /tmp/ifconfig); echo $6)
    local CURR_BYTES=`expr ${RX_BYTES} + ${TX_BYTES}`

    # Calculate the throughput for the last interval (one second).
    local BYTES=`expr ${CURR_BYTES} - ${PREV_BYTES}`
    PREV_BYTES=${CURR_BYTES}
    if [ ${BYTES} -lt 0 ]; then
      echo "${TS} COUNTERS WRAPPED"
      continue
    fi

    # Skip the first measurement.  It's not correct.
    [ ${LOOP_COUNT} = 1 ] && continue

    # Calculate the average bytes per second throughput over our measurement period.
    BYTES_TOTAL=`expr ${BYTES_TOTAL} + ${BYTES}`
    BYTES_TOTAL=`expr ${BYTES_TOTAL} - ${BYTES_ARRAY[${LRU_INDEX}]}`

    # Save the new one second value in the array.
    BYTES_ARRAY[${LRU_INDEX}]=${BYTES}
    LRU_INDEX=`expr ${LRU_INDEX} + 1`
    LRU_INDEX=`expr ${LRU_INDEX} \% ${THROUGHPUT_AVG}`

    # Calculate and display the mbps throughput.
    printf "%5d: %s: %f sec: " ${LOOP_COUNT} "${TS}" ${ELAPSED_TIME_SEC}
    if [ ${THROUGHPUT_MBPS} = 1 ]; then
      local BPS_FLOAT=`expr ${BYTES} \* 8`
      BPS_FLOAT=$(echo "scale=3; ${BPS_FLOAT} / ${ELAPSED_TIME_SEC}" | bc -l)
      local BPS=`printf "%.0f" ${BPS_FLOAT}`
      if [ ${BPS} -lt 1000000 ]; then
        printf "%6d  bps: " ${BPS}
      else
        local MBPS=$(echo "scale=0; ${BPS} / 1000000" | bc -l)
        printf "%6d Mbps: " ${MBPS}
      fi

      local BITS_TOTAL=`expr ${BYTES_TOTAL} \* 8`
      local BPS_TOTAL=`expr ${BITS_TOTAL} \/ ${THROUGHPUT_AVG}`
      if [ ${BPS_TOTAL} -lt 1000000 ]; then
        printf "%6d avg bps.\n" ${BPS_TOTAL}
      else
        local MBPS_TOTAL=$(echo "scale=0; ${BPS_TOTAL} / 1000000" | bc -l)
        printf "%6d avg Mbps.\n" ${MBPS_TOTAL}
      fi

    # Display the total bytes.
    elif [ ${THROUGHPUT_BYTES} = 1 ]; then
        echo "${TS} ${BYTES} Bytes"
    fi

    # Sleep long enough to make a 1 second loop.  It takes a long time to do
    # all of these calculations.  We don't need to sleep for a full second
    # after they're done.
    local DONE_TIME=`date +%s.%N`
    ELAPSED_TIME_SEC=$(echo "scale=10; ${DONE_TIME} - ${CURR_TIME}" | bc -l)
    local ELAPSED_TIME_US=$(echo "scale=10; ${ELAPSED_TIME_SEC} * 1000000" | bc -l)
    local USLEEP_TIME=$(echo "scale=1; 1000000 - ${ELAPSED_TIME_US}" | bc -l)
    USLEEP_TIME=$(echo "scale=1; ${USLEEP_TIME} - 100000" | bc -l) # Adjust a little bit.
    usleep ${USLEEP_TIME}
done
}

###############################################################################
# This function will use statistics from the nfsstat command to calculate the
# number of NFS Calls per second.
###############################################################################
nfs_calls_stats() {
  local PREV_CALLS=0
  local PREV_RETRANSMITS=0

  while [ 1 ]; do

    local TS=`date`
    local LOG=nfsstats.log

    nfsstat -c | head -3 | tail -1 > ${LOG} 2>&1
    local CURR_CALLS=$(set -- $(cat ${LOG}); echo $1)
    local CURR_RETRANSMITS=$(set -- $(cat ${LOG}); echo $2)
    rm -f ${LOG}

    local CALLS=`expr ${CURR_CALLS} - ${PREV_CALLS}`
    PREV_CALLS=${CURR_CALLS}

    local RETRANSMITS=`expr ${CURR_RETRANSMITS} - ${PREV_RETRANSMITS}`
    PREV_RETRANSMITS=${CURR_RETRANSMITS}

    printf "%s  %5d calls: %5d retransmits.\n" "${TS}" ${CALLS} ${RETRANSMITS}

    sleep 1
  done
}

###############################################################################
###############################################################################
########## Start processing here.
###############################################################################
###############################################################################

[ ${DO_THROUGHPUT} = 1 ] && throughput_stats

[ ${DO_NFS_CALLS}  = 1 ] && nfs_calls_stats

# Should never get here.
exit -1

