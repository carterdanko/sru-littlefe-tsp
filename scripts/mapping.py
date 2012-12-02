import os
import sys

# Usage: mapping.py <arg1> <arg2>
#  arg1: the relative path of the data file
#  arg2: the file containing your output
#
# The purpose of this program is to map the tour IDs to their proper
#  id since the GA program simply uses array indices for its IDs.
# After mapping the IDs, the script also submits the tour to the server.
# The format for submission is dependent on specific settings (see ln 68).

#########################################
# GLOBALS
hashmap = list()
tour = list()
filename = ""
#########################################

# Read in the .tsp data file used to construct the input file for GA.
def construct_hash(file):
	global hashmap,filename
	ln = file.readline()
	filename = ln.rstrip('\n').split(": ")[1]
	while (ln!="NODE_COORD_SECTION\n"):
		ln = file.readline()
	# now, you have the first city on next read
	i=0
	while (ln!=''):
		ln = file.readline().rstrip('\n')
		num = ln.split()
		if (len(num)>0):
			hashmap.append( [i,int(num[0])] )
		i+=1

# Read the output file from the GA program.
def read_tour(file):
	global tour
	file.readline()
	nums = file.readline().split("+")
	for s in nums:
		tour.append(int(s))

print 'reading in data file ', sys.argv[1]
f = open(sys.argv[1],'r')


############ MAIN CALLS #################
# Construct hash table
construct_hash(f);
# Print out the hash table
for j in hashmap:
	print j

print 'reading in output file ', sys.argv[2]
f = open(sys.argv[2],'r')
# Read in the tour from the file
read_tour(f)
# Print out the tour
print str(tour)

######## CONFIGURATION ##################
url="http://fedora.cis.cau.edu/tsp/submit.php"
team="sru"
password="password"


submit = "team="+team+"&password="+password+"&set="+filename+"&length=0&tour="
for i in range(0,len(hashmap)-1):
	submit += ""+str(hashmap[tour[i]][1])+"+"
submit +=""+str(hashmap[tour[len(hashmap)-1]][1])
cmd = "curl --data \""+submit+"\" \""+url+"\" -k >out.tmp"
print cmd
os.system(cmd)
cmd = "cat out.tmp | sed -r \"s/(.+ accepted)/`printf \"\\033[32m\"`\\1`printf \"\\033[0m\"`/g\" | sed -r \"s/(((error)|(Incomplete)|(Duplicates)|(Invalid)|(does not)) .+)/`printf \"\\033[31m\"`\\1`printf \"\\033[0m\"`/g\""
os.system(cmd)
cmd = "rm out.tmp"
os.system(cmd)
