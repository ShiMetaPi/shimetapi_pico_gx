#!/bin/sh

stream="-s"
all="-a"
help="-h"
control="-c"
allstop="-as"
CtrlPID=0
StreamPID=0
timeout=0
exitflag=0
DLL_PATH=${LD_LIBRARY_PATH}:${PWD}/libs

if [[ $1 == $allstop ]]; then
killall ittb_control
killall ittb_stream
while true;do
        CtrlPID=`pidof ittb_control`
        StreamPID=`pidof ittb_stream`
        echo "control PID:" $CtrlPID
        echo "stream PID:" $StreamPID
        if [ -z $CtrlPID ] && [ -z $StreamPID ];then
            echo    ">>>>no pid,run it"
            break
        else
            echo ">>>>program is exiting waiting"
            sleep 2
            timeout=`expr $timeout + 1`
        fi
        if [ $timeout -eq 5 ];then
            if [ -z $CtrlPID ];then
                :
            else
                wait $CtrlPID
                kill $CtrlPID
            fi
            if [ -z $StreamPID ];then
                :
            else
                wait $StreamPID
                kill $StreamPID
            fi
            break
        fi
done
exit
elif [ $# -gt 4 ] ;then
echo "**********************************************************************************************************"
echo "1. Full services Usage: ./IspTool.sh -a sensortype index size"
echo "sensortype: type of the sensor."
echo "index: index of mode, refer to the .ini config file, if not provided, the default value is 0."
echo "size: frame size, used for DPC calibration, can be omitted without DPC calibration."
echo " "
echo "2. Only stream service Usage: ./IspTool.sh -s sensortype index"
echo "sensortype: type of the sensor."
echo "index: index of mode, refer to the .ini config file, if not provided, the default value is 0."
echo " "
echo "3. Only control service Usage: ./IspTool.sh -c size"
echo "size: frame size, used for DPC calibration, can be omitted without DPC calibration."
echo " "
echo "4. Stop services Usage: ./IspTool.sh -as"
echo " "
echo "5. Help Usage: ./IspTool.sh -h"
echo " "
echo "NOTE"
echo "All initial configuration files are stored in the release\configs directory."
echo "**********************************************************************************************************"
exit

else
case $1 in
control|$control)
timeout=0
killall ittb_control
killall ittb_stream
while true;do
        CtrlPID=`pidof ittb_control`
        StreamPID=`pidof ittb_stream`
        echo "control PID:" $CtrlPID
        echo "stream PID:" $StreamPID
        if [ -z $CtrlPID ] && [ -z $StreamPID ];then
            echo    ">>>>no pid,run it"
            break
        else
            echo ">>>>program is exiting waiting"
            sleep 2
            timeout=`expr $timeout + 1`
        fi
        if [ $timeout -eq 5 ];then
            if [ -z $CtrlPID ];then
                :
            else
                wait $CtrlPID
                kill $CtrlPID
            fi
            if [ -z $StreamPID ];then
                :
            else
                wait $StreamPID
                kill $StreamPID
            fi
            break
        fi
done
export LD_LIBRARY_PATH=${DLL_PATH}
./ittb_control $2 & exit;;


stream|$stream)
timeout=0
killall ittb_control
killall ittb_stream
while true;do
        CtrlPID=`pidof ittb_control`
        StreamPID=`pidof ittb_stream`
        echo "control PID:" $CtrlPID
        echo "stream PID:" $StreamPID
        if [ -z $CtrlPID ] && [ -z $StreamPID ];then
            echo    ">>>>no pid,run it"
            break
        else
            echo ">>>>program is exiting waiting"
            sleep 2
            timeout=`expr $timeout + 1`
        fi
        if [ $timeout -eq 5 ];then
            if [ -z $CtrlPID ];then
                :
            else
                wait $CtrlPID
                kill $CtrlPID
            fi
            if [ -z $StreamPID ];then
                :
            else
                wait $StreamPID
                kill $StreamPID
            fi
            break
        fi
done
export LD_LIBRARY_PATH=${DLL_PATH}
./ittb_stream $2 $3&  exit;;

all|$all)
killall ittb_control
killall ittb_stream
while true;do
        CtrlPID=`pidof ittb_control`
        StreamPID=`pidof ittb_stream`
        echo "control PID:" $CtrlPID
        echo "stream PID:" $StreamPID
        if [ -z $CtrlPID ] && [ -z $StreamPID ];then
            echo    ">>>>no pid,run it"
            break
        else
            echo ">>>>program is exiting waiting"
            sleep 2
            timeout=`expr $timeout + 1`
        fi
        if [ $timeout -eq 5 ];then
            if [ -z $CtrlPID ];then
                :
            else
                wait $CtrlPID
                kill $CtrlPID
            fi
            if [ -z $StreamPID ];then
                :
            else
                wait $StreamPID
                kill $StreamPID
            fi
            break
        fi
done
export LD_LIBRARY_PATH=${DLL_PATH}
./ittb_stream $2 $3 &
while true;do
    if [ $(ps -T| grep {RecvCfgProc}  |head -1| awk '{print $4}') = "{RecvCfgProc}" ];then
        break;
    fi
    sleep 1
    if [ $(ps | grep ittb_stream |head -1| awk '{print $4}') = "grep" ];then
        exit;
    fi
done
./ittb_control $4 & exit;;
usage|$help)
echo "**********************************************************************************************************"
echo "1. Full services Usage: ./IspTool.sh -a sensortype index size"
echo "sensortype: type of the sensor."
echo "index: index of mode, refer to the .ini config file, if not provided, the default value is 0."
echo "size: frame size, used for DPC calibration, can be omitted without DPC calibration."
echo " "
echo "2. Only stream service Usage: ./IspTool.sh -s sensortype index"
echo "sensortype: type of the sensor."
echo "index: index of mode, refer to the .ini config file, if not provided, the default value is 0."
echo " "
echo "3. Only control service Usage: ./IspTool.sh -c size"
echo "size: frame size, used for DPC calibration, can be omitted without DPC calibration."
echo " "
echo "4. Stop services Usage: ./IspTool.sh -as"
echo " "
echo "5. Help Usage: ./IspTool.sh -h"
echo " "
echo "NOTE"
echo "All initial configuration files are stored in the release\configs directory."
echo "**********************************************************************************************************"
exit;;
esac

fi

echo "**********************************************************************************************************"
echo "1. Full services Usage: ./IspTool.sh -a sensortype index size"
echo "sensortype: type of the sensor."
echo "index: index of mode, refer to the .ini config file, if not provided, the default value is 0."
echo "size: frame size, used for DPC calibration, can be omitted without DPC calibration."
echo " "
echo "2. Only stream service Usage: ./IspTool.sh -s sensortype index"
echo "sensortype: type of the sensor."
echo "index: index of mode, refer to the .ini config file, if not provided, the default value is 0."
echo " "
echo "3. Only control service Usage: ./IspTool.sh -c size"
echo "size: frame size, used for DPC calibration, can be omitted without DPC calibration."
echo " "
echo "4. Stop services Usage: ./IspTool.sh -as"
echo " "
echo "5. Help Usage: ./IspTool.sh -h"
echo " "
echo "NOTE"
echo "All initial configuration files are stored in the release\configs directory."
echo "**********************************************************************************************************"
