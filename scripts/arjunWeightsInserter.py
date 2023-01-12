import sys

# for weighted CNFs, I ran arjun with --renumber 0 to keep variable list the same. But arjun does not preserve weight lines "c p weight "
# so have to reinsert those lines back into the cnf files
# also some original cnfs (including unweighted) could not be preprocessed by arjun in the timeout and so are missing in the arjun folder
# this script takes the original cnf file cIF, corresponding arjun-preprocessed file aIF , and writes a CNF file oF which is the aIF appended wiht c p weight lines from cIF
# if the file is missing it simply copies the original cnf and adds a comment that arjun failed. c c arjun was unable to finish processing. This is the original file.
# hardcoding paths etc since this is a one time operation for now


cIDir1 = "/home/adi/Downloads/prob_inf/benchmarks/cnf/mcc-21_benchmarks/track1_private/track1_"
cIDir2 = "/home/adi/Downloads/prob_inf/benchmarks/cnf/mcc-21_benchmarks/track2_private/track2_"
cIDir3 = "/home/adi/Downloads/prob_inf/benchmarks/cnf/mcc-22_benchmarks/MC2022_track1-mc_public/mc2022_track1_"
cIDir4 = "/home/adi/Downloads/prob_inf/benchmarks/cnf/mcc-22_benchmarks/MC2022_track2-wmc_public/mc2022_track2_"

aDir = "/home/adi/Downloads/prob_inf/benchmarks/cnf/arjun/arjun_array_"

oDir = "/home/adi/Downloads/prob_inf/benchmarks/cnf/arjun_with_weights_and_missing-files_temp/arjun_array_"

def getWeightLines(f):
	nonWtLines = ''
	wtLines = ''
	for line in f:
		if line.startswith('c p weight '):
			wtLines += line
		else:
			nonWtLines += line
	return wtLines, nonWtLines

for i in range(400):
	if int(i/100) == 0:
		fname = cIDir1 + str(i*2).zfill(3) + ".cnf"
	elif int(i/100) == 1:
		fname = cIDir2 + str((i-100)*2).zfill(3) + ".cnf"
	elif int(i/100) == 2:
		fname = cIDir3 + str((i-200)*2+1).zfill(3) + ".cnf"
	elif int(i/100) == 3:
		fname = cIDir4 + str((i-300)*2+1).zfill(3) + ".cnf"
	cIF = open(fname,'r')
	wtLines, nonWtLines = getWeightLines(cIF)
	cIF.close()
	fname = aDir + str(i) + ".cnf"
	aLines = ""
	missingfilecomment = False
	try:
		aIF = open(fname,'r')
		aLines = aIF.readlines()
		aIF.close()
	except OSError as e:
		print(f"Unable to open {fname}: {e}", file=sys.stderr)
		aLines = nonWtLines
		missingfilecomment = True
	#fname = oDir + str(int(i+100*(1+(int(i/100))))) + ".cnf"
	fname = oDir + str(i) + ".cnf"
	oF = open(fname,'w')
	for line in aLines:
		oF.write(line)
	for line in wtLines:
		oF.write(line)
	if missingfilecomment:
		oF.write('c c arjun was unable to finish processing')
	oF.close()
	
