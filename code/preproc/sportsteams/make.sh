#!/bin/bash
set -eux
python clean.py | sort | uniq > sportsteams.txt
