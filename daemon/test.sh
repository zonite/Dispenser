#!/bin/bash

slot00_up=$( gpioget gpiochip0 22 )
slot01_up=$( gpioget gpiochip0 23 )
slot02_up=$( gpioget gpiochip0 24 )
slot10_up=$( gpioget gpiochip0 18 )
slot11_up=$( gpioget gpiochip0 17 )
slot12_up=$( gpioget gpiochip0 16 )

slot00_down=$( gpioget -l gpiochip0 25 )
slot01_down=$( gpioget -l gpiochip0 26 )
slot02_down=$( gpioget -l gpiochip0 27 )
slot10_down=$( gpioget -l gpiochip0 21 )
slot11_down=$( gpioget -l gpiochip0 20 )
slot12_down=$( gpioget -l gpiochip0 19 )

#slot00_release= $(gpioget gpiochip0 5)
#slot01_release= $(gpioget gpiochip0 6)
#slot02_release= $(gpioget gpiochip0 7)
#slot10_release= $(gpioget gpiochip0 4)
#slot11_release= $(gpioget gpiochip0 1)
#slot12_release= $(gpioget gpiochip0 0)

#echo "slot 00 up=${slot00_up} down=${slot00_down} release=${slot00_release}"
#echo "slot 01 up=${slot01_up} down=${slot01_down} release=${slot01_release}"
#echo "slot 02 up=${slot02_up} down=${slot02_down} release=${slot02_release}"
#echo "slot 10 up=${slot10_up} down=${slot10_down} release=${slot10_release}"
#echo "slot 11 up=${slot11_up} down=${slot11_down} release=${slot11_release}"
#echo "slot 12 up=${slot12_up} down=${slot12_down} release=${slot12_release}"

echo "slot 00 up=${slot00_up} down=${slot00_down}"
echo "slot 01 up=${slot01_up} down=${slot01_down}"
echo "slot 02 up=${slot02_up} down=${slot02_down}"
echo "slot 10 up=${slot10_up} down=${slot10_down}"
echo "slot 11 up=${slot11_up} down=${slot11_down}"
echo "slot 12 up=${slot12_up} down=${slot12_down}"

