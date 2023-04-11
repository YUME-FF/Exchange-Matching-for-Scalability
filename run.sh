CoreNum="1"
threadpoolsize="1"
# if has arg[2]
if [ -n "$2" ]
then
  CoreNum=$2
fi

# if has arg[3] and arg[3]>0
if [ -n "$3" ] && [ "$3" -gt  0 ]; then
  threadpoolsize=$3
fi
#core should be [0 - 4]
last=$((${CoreNum}-1));
if [ ${last} != "-1" ] && [ ${last} != "0" ] && [ ${last} != "1" ] && [ ${last} != "2" ] && [ ${last} != "3" ]
then
  echo "Invalid input"
  exit 1
fi

cd src/server
echo "Now making file..."
make clean
make

if [ ${last} == -1 ]; then
    ./main
elif [ ${last} == 0 ]; then
  echo "Your threadpool size has been set to ${threadpoolsize}"
  if [ -n "$1" ]
  then
    echo "$1"
    taskset -c 0 ./main "$1" ${threadpoolsize}
  else
    taskset -c 0 ./main localhost ${threadpoolsize}
  fi
else
  echo "You are using ${CoreNum} cores"
  echo "Your threadpool size has been set to ${threadpoolsize}"
  if [ -n "$1" ]; then
    taskset -c 0-${last} ./main "$1" ${threadpoolsize}
  else
    taskset -c 0-${last} ./main localhost ${threadpoolsize}
  fi
fi