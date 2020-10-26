#!/usr/bin/sh
while true
do
    swaymsg -- seat seat0 cursor move 20 0
    sleep 0.01
    swaymsg -- seat seat0 cursor move 20 0
    sleep 0.01
    swaymsg -- seat seat0 cursor move 20 0
    sleep 0.01
    swaymsg -- seat seat0 cursor move 20 0
    sleep 0.01
    swaymsg -- seat seat0 cursor move 0 20
    sleep 0.01
    swaymsg -- seat seat0 cursor move 0 20
    sleep 0.01
    swaymsg -- seat seat0 cursor move 0 20
    sleep 0.01
    swaymsg -- seat seat0 cursor move 0 20
    sleep 0.01
    swaymsg -- seat seat0 cursor move -20 0
    sleep 0.01
    swaymsg -- seat seat0 cursor move -20 0
    sleep 0.01
    swaymsg -- seat seat0 cursor move -20 0
    sleep 0.01
    swaymsg -- seat seat0 cursor move -20 0
    sleep 0.01
    swaymsg -- seat seat0 cursor move 0 -20
    sleep 0.01
    swaymsg -- seat seat0 cursor move 0 -20
    sleep 0.01
    swaymsg -- seat seat0 cursor move 0 -20
    sleep 0.01
    swaymsg -- seat seat0 cursor move 0 -20
    sleep 0.01
done
