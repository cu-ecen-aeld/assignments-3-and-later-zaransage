#!/bin/sh

case "$1" in
  start)
  echo "Starting aesdsocket server"
  /usr/bin/server
  ;;
  stop)
  echo "Stopping aesdsocket server"
  kill -2 /usr/bin/server
  ;;