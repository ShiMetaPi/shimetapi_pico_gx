export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PWD}/libs
killall ittb_control
kill -9 `ps | grep control |head -1| awk '{print $1}'`;
./ittb_control $1 &
