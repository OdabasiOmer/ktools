#!/bin/sh

# test gulcalc item stream and coverage stream
eve 1 1 | getmodel | gulcalc -S1 -a0 -i ./out/gulcalci.bin
gultocsv < ./out/gulcalci.bin > ./out/gulcalci.csv 

# test fmcalc
fmcalc < ./out/gulcalci.bin > ./out/fmcalc.bin
fmtocsv > ./out/fmcalci.csv < ./out/fmcalc.bin

# test summarycalc
summarycalc -i -2 /out/gulsummarycalc2.bin   < ./out/gulcalci.bin
summarycalc -f -1 ./out/fmsummarycalc1.bin < ./out/fmcalc.bin

# test selt
summarycalctocsv -o > ./out/gulselt1.csv < ./out/gulsummarycalc1.bin

# test eltcalc (prints out csv)
eltcalc < ./out//gulsummarycalc2.bin > ./out/gulelt2.csv
eltcalc < ./out/fmsummarycalc1.bin > ./out/fmelt1.csv
