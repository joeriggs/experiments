#!/bin/sh

###############################################################################
# WARNING: This tool uses the linux "date" program to calculate dates, so it is
# susceptible to the 2038 date problem.
###############################################################################

########################################
# TARGET_BEG[] contains the starting dates we're counting down from (TARGET_DISP = P).
# TARGET_END[] contains the end dates we're counting down to.
# TARGET_DISP[] contains the counting scale:
#   Y = Years.
#   W = Weeks.
#   D = Days.
#   H = Hours.
#   A = Auto-pick the best scale (Y -> W -> D -> H -> M -> S).
#   C = Clock (DDDDD:HH:MM:SS).
#   P = Percentage from start to finish.
########################################
TARGET_BEG[${#TARGET_BEG[@]}]="";                    TARGET_END[${#TARGET_END[@]}]="2022-04-04 00:00:00"; TARGET_DISP[${#TARGET_DISP[@]}]="D"
TARGET_BEG[${#TARGET_BEG[@]}]="";                    TARGET_END[${#TARGET_END[@]}]="2022-04-04 00:00:00"; TARGET_DISP[${#TARGET_DISP[@]}]="W"
TARGET_BEG[${#TARGET_BEG[@]}]="2020-04-04 00:00:00"; TARGET_END[${#TARGET_END[@]}]="2022-04-04 00:00:00"; TARGET_DISP[${#TARGET_DISP[@]}]="P"
TARGET_BEG[${#TARGET_BEG[@]}]="";                    TARGET_END[${#TARGET_END[@]}]="2021-04-04 00:00:00"; TARGET_DISP[${#TARGET_DISP[@]}]="D"
TARGET_BEG[${#TARGET_BEG[@]}]="";                    TARGET_END[${#TARGET_END[@]}]="2021-04-04 00:00:00"; TARGET_DISP[${#TARGET_DISP[@]}]="W"
TARGET_BEG[${#TARGET_BEG[@]}]="2020-04-04 00:00:00"; TARGET_END[${#TARGET_END[@]}]="2021-04-04 00:00:00"; TARGET_DISP[${#TARGET_DISP[@]}]="P"
TARGET_END_COUNT=${#TARGET_END[@]}

echo ${TARGET_END}

########################################
# Constants.  DO NOT MODIFY.
SECS_PER_MN=60
SECS_PER_HR=3600
SECS_PER_DY=86400    # Not accounting for DST.
SECS_PER_WK=604800   # Not accounting for DST.
SECS_PER_YR=31536000 # Not accounting for leap years.

###############################################################################
###############################################################################
############## PRIVATE FUNCTION(S).
###############################################################################
###############################################################################

########################################
# This function converts a date to a number of seconds.
#
# Input: DATE = The date to convert.  It MUST be formatted as follows:
#                    YYYY-MM-DD HH:MM:SS
#               The script is entirely unforgiving if you don't use this format.
#
# Output: CALCULATED_SECONDS = The number of seconds.
#
CALCULATED_SECONDS=0
function date_to_seconds() {
  local DATE="${1}"
  CALCULATED_SECONDS=`date -d "${DATE}" +%s`
}


###############################################################################
###############################################################################
############## START EXECUTION HERE.
###############################################################################
###############################################################################

TARGET_END_INDEX=0
printf "Counting down to:\n"
while [ ${TARGET_END_INDEX} -lt ${TARGET_END_COUNT} ]; do
  printf "  %s\n" "${TARGET_END[${TARGET_END_INDEX}]}"
  TARGET_END_INDEX=`expr ${TARGET_END_INDEX} + 1`
done
printf "Press <Enter> to continue.  Press <Ctrl-C> to quit. "
read

########################################
# Print a header.
TARGET_BEG_INDEX=0
printf " | "
while [ ${TARGET_BEG_INDEX} -lt ${TARGET_END_COUNT} ]; do
  TARGET_BEG_STRING="${TARGET_BEG[${TARGET_BEG_INDEX}]}"
  [ -z "${TARGET_BEG_STRING}" ] && TARGET_BEG_STRING="                   "
  printf "   %s | " "${TARGET_BEG_STRING}"
  TARGET_BEG_INDEX=`expr ${TARGET_BEG_INDEX} + 1`
done
printf "                    |\n"

TARGET_END_INDEX=0
printf " | "
while [ ${TARGET_END_INDEX} -lt ${TARGET_END_COUNT} ]; do
  printf "   %s | " "${TARGET_END[${TARGET_END_INDEX}]}"
  TARGET_END_INDEX=`expr ${TARGET_END_INDEX} + 1`
done
printf "======= NOW ======= |\n"

########################################
# Cycle through the list of dates and convert them to seconds.  The number of
# seconds will never change, so we'll only calculate them once.
TARGET_END_INDEX=0
while [ ${TARGET_END_INDEX} -lt ${TARGET_END_COUNT} ]; do
  date_to_seconds "${TARGET_BEG[${TARGET_END_INDEX}]}"
  TARG_BEG_SECS[${TARGET_END_INDEX}]=${CALCULATED_SECONDS}
  date_to_seconds "${TARGET_END[${TARGET_END_INDEX}]}"
  TARG_END_SECS[${TARGET_END_INDEX}]=${CALCULATED_SECONDS}
  TARGET_END_INDEX=`expr ${TARGET_END_INDEX} + 1`
done

########################################
# Cycle through the list of dates and update the counters.
while [ 1 ]; do
  sleep 1

  NOW=`date +"%Y-%m-%d %H:%M:%S"`
  date_to_seconds "${NOW}"
  CURR_SECS=${CALCULATED_SECONDS}

  TARGET_END_INDEX=0
  while [ ${TARGET_END_INDEX} -lt ${TARGET_END_COUNT} ]; do
    DIFF=`expr ${TARG_END_SECS[${TARGET_END_INDEX}]} - ${CURR_SECS}`

    case ${TARGET_DISP[${TARGET_END_INDEX}]} in

    "Y")
      UNIT_NAME="YEARS"
      FRACTION=$(echo "scale=10; ${DIFF} / ${SECS_PER_YR}" | bc -l)
      PRINTF_VAL=${FRACTION}
      PRINTF_FMT="%15.8f %s"
      ;;

    "W")
      UNIT_NAME="WEEKS"
      FRACTION=$(echo "scale=10; ${DIFF} / ${SECS_PER_WK}" | bc -l)
      PRINTF_VAL=${FRACTION}
      PRINTF_FMT="%15.6f %s"
      ;;

    "D")
      UNIT_NAME="DAYS "
      FRACTION=$(echo "scale=10; ${DIFF} / ${SECS_PER_DY}" | bc -l)
      PRINTF_VAL=${FRACTION}
      PRINTF_FMT="%15.5f %s"
      ;;

    "H")
      UNIT_NAME="HOURS"
      FRACTION=$(echo "scale=10; ${DIFF} / ${SECS_PER_HR}" | bc -l)
      PRINTF_VAL=${FRACTION}
      PRINTF_FMT="%15.4f %s"
      ;;

    "A")
      if [ ${DIFF} -lt ${SECS_PER_MN} ]; then
        UNIT_NAME="SECS "
        FRACTION=$(echo "scale=10; ${DIFF} / 1             " | bc -l)
        PRINTF_FMT="%15.0f %s"
      elif [ ${DIFF} -lt ${SECS_PER_HR} ]; then
        UNIT_NAME="MINS "
        FRACTION=$(echo "scale=10; ${DIFF} / ${SECS_PER_MN}" | bc -l)
        PRINTF_FMT="%15.2f %s"
      elif [ ${DIFF} -lt ${SECS_PER_DY} ]; then
        UNIT_NAME="HOURS"
        FRACTION=$(echo "scale=10; ${DIFF} / ${SECS_PER_HR}" | bc -l)
        PRINTF_FMT="%15.4f %s"
      elif [ ${DIFF} -lt ${SECS_PER_WK} ]; then
        UNIT_NAME="DAYS "
        FRACTION=$(echo "scale=10; ${DIFF} / ${SECS_PER_DY}" | bc -l)
        PRINTF_FMT="%15.5f %s"
      elif [ ${DIFF} -lt ${SECS_PER_YR} ]; then
        UNIT_NAME="WEEKS"
        FRACTION=$(echo "scale=10; ${DIFF} / ${SECS_PER_WK}" | bc -l)
        PRINTF_FMT="%15.6f %s"
      else
        UNIT_NAME="YEARS"
        FRACTION=$(echo "scale=10; ${DIFF} / ${SECS_PER_YR}" | bc -l)
        PRINTF_FMT="%15.8f %s"
      fi
      PRINTF_VAL=${FRACTION}
      ;;

    "C")
      UNIT_NAME=""
      FRACTION=$(echo "scale=10; ${DIFF} / 1" | bc -l)
      LT_ZERO=$(echo "${FRACTION} < 0.0" | bc -l)
      if [ ${LT_ZERO} = 0 ]; then
        DY=`expr ${DIFF} / ${SECS_PER_DY}`
        DIFF=`expr ${DIFF} % ${SECS_PER_DY}`
        HR=`expr ${DIFF} / ${SECS_PER_HR}`
        DIFF=`expr ${DIFF} % ${SECS_PER_HR}`
        MN=`expr ${DIFF} / ${SECS_PER_MN}`
        DIFF=`expr ${DIFF} % ${SECS_PER_MN}`
        if [ ${DY} -gt 0 ]; then
          PRINTF_VAL=`printf "%6d:%02d:%02d:%02d" ${DY} ${HR} ${MN} ${DIFF}`
          PRINTF_FMT="          %s %s"
          [ ${DY} -gt    9 ] && PRINTF_FMT="         %s %s"
          [ ${DY} -gt   99 ] && PRINTF_FMT="        %s %s"
          [ ${DY} -gt  999 ] && PRINTF_FMT="       %s %s"
          [ ${DY} -gt 9999 ] && PRINTF_FMT="      %s %s"
        elif [ ${HR} -gt 0 ]; then
          PRINTF_VAL=`printf "%2d:%02d:%02d" ${HR} ${MN} ${DIFF}`
          PRINTF_FMT="             %s %s"
          [ ${HR} -gt   9 ] && PRINTF_FMT="            %s %s"
        elif [ ${MN} -gt 0 ]; then
          PRINTF_VAL=`printf "%12d:%02d" ${MN} ${DIFF}`
          PRINTF_FMT="                %s %s"
          [ ${MN} -gt   9 ] && PRINTF_FMT="               %s %s"
        else
          PRINTF_VAL=`printf "%15d" ${DIFF}`
          PRINTF_FMT="                   %s %s"
          [ ${DIFF} -gt   9 ] && PRINTF_FMT="                  %s %s"
        fi
      fi
      ;;

    "P")
      BEG_SECS=${TARG_BEG_SECS[${TARGET_END_INDEX}]}
      END_SECS=${TARG_END_SECS[${TARGET_END_INDEX}]}
      if [ ${CURR_SECS} -lt ${BEG_SECS} ]; then
        FRACTION=0
        UNIT_NAME=" "
        PRINTF_VAL="BEFORE"
        PRINTF_FMT="             %s %s"
      elif [ ${CURR_SECS} -gt ${END_SECS} ]; then
        FRACTION=-1
      else
        UNIT_NAME="%"
        TOT_SECS=`expr ${END_SECS} - ${BEG_SECS}`
        ELAPSED_SECS=`expr ${CURR_SECS} - ${BEG_SECS}`
        FRACTION=$(echo "scale=10; ( ${ELAPSED_SECS} / ${TOT_SECS} ) * 100" | bc -l)
        PRINTF_VAL=${FRACTION}
        PRINTF_FMT="%19.9f %s"
      fi
      ;;

    *)
      echo "ERROR"
      exit 1
      ;;

    esac

    LT_ZERO=$(echo "${FRACTION} < 0.0" | bc -l)
    if [ ${LT_ZERO} = 1 ]; then
      printf " |                  DONE "
    else
      printf " |  ${PRINTF_FMT}" ${PRINTF_VAL} "${UNIT_NAME}"
    fi

    TARGET_END_INDEX=`expr ${TARGET_END_INDEX} + 1`
  done
  printf " | %s |\r" "${NOW}"
done

