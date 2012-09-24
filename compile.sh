#! /bin/sh
make -n > temp.sh
dos2unix temp.sh
sh -x ./temp.sh
rm temp.sh

