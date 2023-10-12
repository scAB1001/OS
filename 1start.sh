#!/bin/bash

singularity pull docker://callaghanmt/xv6-tools:buildx-latest

singularity shell xv6-tools_buildx-latest.sif 

