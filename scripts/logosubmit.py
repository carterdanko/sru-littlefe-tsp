import os
import sys

## system args
# arg1: the relative path of the data file
# arg2: the relative path of the output file from LOGO
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
def construct_tour(file2):
	global tour,citysize
	ln = file2.readline().rstrip('\n')
	while (ln!=''):
		tour.append(int(ln))
		ln = file2.readline().rstrip('\n')
	# enddef


############ MAIN CALLS #################
f = open(sys.argv[1],'r')
read_file(f)

f = open(sys.argv[2],'r')
construct_tour(f)

# Print out the tour
print str(tour)

######## CONFIGURATION ##################
url="https://fedora.cis.cau.edu/tsp/submit.php"
team="sru"
password="password"


submit = "team="+team+"&password="+password+"&set="+filename+"&length=0&tour="
submit += str(tour[0])
for i in range(1,citysize):
	submit += "+"+str(tour[i])
cmd = "curl --data \""+submit+"\" \""+url+"\" -k >out.tmp"
print cmd
os.system(cmd)
# cmd = "cat out.tmp"
cmd = "cat out.tmp | sed -r \"s/(.+ accepted)/`printf \"\\033[32m\"`\\1`printf \"\\033[0m\"`/g\" | sed -r \"s/(((error)|(Incomplete)|(Duplicates)|(Invalid)|(does not)) .+)/`printf \"\\033[31m\"`\\1`printf \"\\033[0m\"`/g\""
os.system(cmd)
cmd = "rm out.tmp"
os.system(cmd)
