#!/bin/bash

##################################
# Submits results to the server
# usage:
#     first argument is the data set
#     second argument is the tours file


######### COMMAND LINE ARGS #############
dataSet=$1
file=$2

######## CONFIGURATION ##################
url="http://fedora.cis.cau.edu/tsp/submit.php"
team="sru"
password="password"

############## INPUT GENERATION ########
size=`cat $file | head -n 1 | sed -r "s/[0-9]+ ([0-9]+)/\1/g"`
tour=`cat $file | head -n 2 | tail -n 1`
outTour=`echo $tour | head -c 40`


########## DEBUG OUT ###############
echo "Size: $size"
echo "Tour(condensed): $outTour..."


######## DO THE SUBMISSION #############
a="team=$team&password=$password&set=$dataSet&length=0&tour=$tour"
#wget "$url?$a" --no-check-certificate -O "out.tmp"
curl --data $a $url -k > out.tmp
cat out.tmp | sed -r "s/(.+ accepted)/`printf "\033[32m"`\1`printf "\033[0m"`/g" | sed -r "s/(((error)|(Incomplete)|(Duplicates)|(Invalid)|(does not)) .+)/`printf "\033[31m"`\1`printf "\033[0m"`/g"
rm out.tmp


echo "submission done."

