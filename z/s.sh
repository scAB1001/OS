#!/bin/bash
clear

#gcc ./z.c -o test
gcc ./x.c -o test

./test "   echo 'Hi ho'  "

rm test