#!/bin/sh

PID="/var/run/aesdsocket.pid"

case "$1" in
  start)
  echo "Starting aesdsocket server"
  /usr/bin/aesdsocket -d && echo $! > "${PID}"
  ;;
  stop)
  echo "Stopping aesdsocket server"
  if [ -f "${PID}" ]; then kill $(cat ${PID}); rm -f "${PID}"; fi
  ;;
  *)
  echo "Call either start or stop"
  exit 1
esac