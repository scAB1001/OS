#!/bin/bash

gcc ./mock_shell.c -o mock_shell
clear
./mock_shell "   echo 'Hi ho'  "
rm mock_shell