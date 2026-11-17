#!/bin/sh
cd . && echo \
@goto cmd \
>/dev/null

# posix shell code
dir=$(dirname "$0")
if [ $1 ]; then
  if cmake "$@" "$dir"; then
    echo
    echo Running $(basename "$0") will allow you to fine-tune configuration options.
  fi
elif which cmake-gui; then
  cmake-gui -S "$dir" -B "$dir"
elif which ccmake; then
  ccmake -S "$dir" -B "$dir"
else
  cmake -i -S "$dir" -B "$dir"
fi
exit

# windows cmd code
:cmd
@echo off
cls
if "%1"=="" (
  start "" cmake-gui -S "%~dp0." -B "%~dp0."
) else (
  cmake %* -S "%~dp0." -B "%~dp0."
  echo.
  echo Running %~nx0 will allow you to fine-tune configuration options.
  pause
)
