import os
import sys

## system args
# arg1: the relative path of the data file
# arg2: handles which type of tour to construct (1=sequential, 2=interlaced)
##

#########################################
# GLOBALS
tour = list()
citysize = 0
filename = ""
#########################################

def read_file(file):
	global filename,citysize
	ln = file.readline()
	filename = ln.rstrip('\n').split(": ")[1]
	while (ln!="TYPE : TSP\n"):
		ln = file.readline()
	# now, you have city size on next read
	ln = file.readline()
	citysize = int(ln.rstrip('\n').split(": ")[1])

# the var 'x' determines what type of tour you construct
def construct_tour(x):
	global tour,citysize
	if x==2:
		if citysize%2==1:
			y=0
			for i in range(1,citysize+1):
				tour.append(y+1)
				y=(y+2)%citysize
		else:
			for i in range(0,citysize):
				tour.append((i*2+1)%(citysize+1)+1)
	else:
		for i in range(1,citysize+1):
			tour.append(i)
	# enddef


############ MAIN CALLS #################
print 'reading in data file ', sys.argv[1]
f = open(sys.argv[1],'r')
read_file(f)
print 'citysize is ', citysize

construct_tour( int(sys.argv[2]) )

# Print out the tour
print str(tour)

######## CONFIGURATION ##################
url="https://fedora.cis.cau.edu/tsp/submit.php"
team="sru"
password="password"


submit = "team="+team+"&password="+password+"&set="+filename+"&length=0&tour="
for i in range(0,citysize):
	submit += ""+str(tour[i])+"+"
cmd = "curl --data \""+submit+"\" \""+url+"\" -k >out.tmp"
print cmd
os.system(cmd)
# cmd = "cat out.tmp"
cmd = "cat out.tmp | sed -r \"s/(.+ accepted)/`printf \"\\033[32m\"`\\1`printf \"\\033[0m\"`/g\" | sed -r \"s/(((error)|(Incomplete)|(Duplicates)|(Invalid)|(does not)) .+)/`printf \"\\033[31m\"`\\1`printf \"\\033[0m\"`/g\""
os.system(cmd)
cmd = "rm out.tmp"
os.system(cmd)
