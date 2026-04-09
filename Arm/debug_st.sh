#!/bin/sh

# Cleaning previous build
make clean
make rebuild "OPT=-g -O0"

if [ ! -f "./bin/bulk_usb.elf" ]; then
    echo "Error: Build failed - bulk_usb.elf not found!"
    exit 1
fi

# Terminal cleanup
cleanup() {
    echo "Cleaning up debug session..."
    tmux kill-session -t stm32-debug 2>/dev/null
    pkill -f "st-util" 2>/dev/null
}

# Cleanup setup
trap cleanup EXIT INT TERM

# Запускаем отладку
tmux new-session -d -s stm32-debug
tmux split-window -h
tmux send-keys -t 1 "st-util -p 4242" Enter
sleep 3

tmux send-keys -t 0 "/opt/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gdb -iex \"set auto-load safe-path /\" -tui ./bin/bulk_usb.elf" Enter
tmux send-keys -t 0 "target extended-remote :4242" Enter
tmux send-keys -t 0 "monitor reset halt" Enter
tmux send-keys -t 0 "load" Enter
tmux send-keys -t 0 "tbreak main" Enter
tmux send-keys -t 0 "continue" Enter

# Waiting GDB termination
tmux attach -t stm32-debug

# removing session
cleanup
