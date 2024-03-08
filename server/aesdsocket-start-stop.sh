#!/bin/sh

PID="/var/run/aesdsocket.pid"

case "$1" in
  start)
    echo "Starting aesdsocket server"
    /usr/bin/aesdsocket -d
  ;;
  stop)
    echo "Stopping aesdsocket server"
    if [ -f "${PID}" ]; then
      kill $(cat ${PID})
      rm -f "${PID}"
    else
      echo "aesdsocket server not running"
    fi
  ;;
  *)
    echo "start|stop"
    exit 1
esac
