import sys, os, math
import numpy
from scipy import stats
import pylab as pl

fpath = sys.argv[1]
numSamples = int(sys.argv[2])
skipTo = None
skipList = [False]*5
if len(sys.argv) >= 4:
	skipTo = int(sys.argv[3])
	for i in range(min(4,skipTo)):
		skipList[i] = True
assert(os.environ.has_key('TMP'))
assert(os.environ.has_key('DPSAMPLER'))
assert(os.environ.has_key('WAPS'))

fname = os.path.basename(fpath)
fpath = os.path.abspath(fpath)
DPSampleFpath = os.environ['TMP']+'/'+fname+'.dpsamples'
idealSampleFpath = os.environ['TMP']+'/'+fname+'.idealsamples'

#generate samples from dpmc
if not skipList[0]:
	cmd = 'cd ..; python '+os.environ['DPSAMPLER']+'/runDMC.py '+fpath+' s '+str(numSamples)+' -1 '+DPSampleFpath
	print 'Generating samples from dpmc:\n'+cmd
	os.system(cmd)

#gen samples from ideal
if not skipList[1]: 
	cmd = 'python '+ os.environ['WAPS']+'/waps.py '+fpath+' --outputfile '+idealSampleFpath+' --samples '+str(numSamples)
	print 'Generating CNF samples..\n'+cmd
	os.system(cmd)

if skipTo==2:
	print 'Skip to argument is 2. So assuming sample files are already and computing their distributions..'
	DPSampleFpath = os.environ['TMP']+'/'+fname+'.dpsamples'
	idealSampleFpath = os.environ['TMP']+'/'+fname+'.idealsamples'

def processDPSampleFile(sf_):
	models={}
	sfHeader = sf_.readline().split()
	origFname, nVars, numSamples = (sfHeader[i] if i==0 else int(sfHeader[i]) for i in range(len(sfHeader)))
	for line in sf_:
		model = ''.join(line.split())
		if model in models:
			models[model] += 1
		else:
			models[model] = 1
	sf_.close()
	return models

def processWAPSSampleFile(cf_):
	idealmodels = {}
	i = 1
	for line in cf_:
		ls = line.split()
		assert(ls[0]==str(i)+',')
		model = ''.join('0' if int(ls[j])==-j else '1' if int(ls[j])==j else None for j in range(1,len(ls)))
		if model in idealmodels:
			idealmodels[model] += 1
		else:
			idealmodels[model] = 1
		i += 1
	cf_.close()
	return idealmodels

#compute model counts of dd samples 
sf = open(DPSampleFpath,'r')
dpmodels = None
firstline = sf.readline()
if (firstline.startswith('1,')):
	sf.seek(0,0)
	dpmodels = processWAPSSampleFile(sf)
else:
	sf.seek(0,0)
	dpmodels = processDPSampleFile(sf)


#compute model counts of cnf samples
cf = open(idealSampleFpath,'r')
idealmodels = None
firstline = cf.readline()
if (firstline.startswith('1,')):
	cf.seek(0,0)
	idealmodels = processWAPSSampleFile(cf)
else:
	cf.seek(0,0)
	idealmodels = processDPSampleFile(cf)

#compute frequencies
sk = dpmodels.keys()
print len(sk),' models found in dp samples'
ck = idealmodels.keys()
print len(ck),' models found in ideal samples'
print 'Example models:\n'
print 'DPSAmple:',sk[0],'\n',sk[1]
print '\nIDEAL:',ck[0],'\n',ck[1]
s1 = set(sk)
s2 = set(ck)
s3 = s2.union(s1)
print len(s3),' models found in union'

counts = [[0,0]]
for key in s3:
	#for bdd
	if key not in sk:
		counts[0][0] += 1
	else:
		freq = dpmodels[key]
		while len(counts) < freq + 1:
			counts.append([0,0])
		counts[freq][0] += 1
	#for cnf
	if key not in ck:
		counts[0][1] += 1
	else:
		freq = idealmodels[key]
		while len(counts) < freq + 1:
			counts.append([0,0])
		counts[freq][1] += 1
'''
#Switched counts[i][0] and counts[i][1] to test if computed Jensen Shannon is symmetric
for key in s3:
	#for bdd
	if key not in sk:
		counts[0][1] += 1
	else:
		freq = bddmodels[key]
		while len(counts) < freq + 1:
			counts.append([0,0])
		counts[freq][1] += 1
	#for cnf
	if key not in ck:
		counts[0][0] += 1
	else:
		freq = cnfmodels[key]
		while len(counts) < freq + 1:
			counts.append([0,0])
		counts[freq][0] += 1
'''
# normalize to get probs
sum0 = sum(counts[i][0] for i in range(len(counts))) + 0.0
sum1 = sum(counts[i][1] for i in range(len(counts))) + 0.0
#print 'Sum0:',sum0,'Sum1:',sum1
#datapoints check
dp1 = sum(i*counts[i][0] for i in range(len(counts)))
dp2 = sum(i*counts[i][1] for i in range(len(counts)))

print "number of datapoints1:",dp1,"datapoints2:",dp2,"numSamples:",numSamples

for i in range(len(counts)):
	counts[i][0] = counts[i][0] / sum0
	counts[i][1] = counts[i][1] / sum1
#compute jensen shannon
# calculate the kl divergence
def js_divergence(p):
	return                                           0.5*sum(p[i][0] * math.log(p[i][0]/((p[i][0]+p[i][1])/2.0),2) 
	if p[i][0] != 0 else 0 for i in range(len(p))) + 0.5*sum(p[i][1] * math.log(p[i][1]/((p[i][0]+p[i][1])/2.0),2) 
	if p[i][1] != 0 else 0 for i in range(len(p)))
print 'Jensen Shannon Distance is',js_divergence(counts),'bits'

#plot graphs
colors=['green','blue','brown','orange']
mkrs= ['o','^','*','s','D']
patterns = ('-', '\\\\', 'o', 'x', '*', '+', '.', 'O')
mss=10
bar_width = 1.0

#lb = 70
#ub = 170
lb=0
ub=len(counts)
counts0 = [sum0*counts[i][0] for i in range(lb,ub)]
counts1 = [sum1*counts[i][1] for i in range(lb,ub)]
p1 = pl.scatter([i for i in range(lb,ub)],counts0,color='g',marker='x')
p2 = pl.scatter([i for i in range(lb,ub)],counts1,color='r',marker='+')
pl.rcParams['pdf.fonttype'] = 42
pl.rcParams['ps.fonttype'] = 42			
pl.legend((p1,p2),('Trace Sampler', 'Ideal Sampler'),loc='best',fontsize=12)
pl.tick_params(labelsize=16)
#pl.ylim(0,505)
#pl.xlim(0,505)
pl.xlabel('Count', fontsize=20)
pl.ylabel('# of Solutions', fontsize=20)
#pl.yscale('log')
#pl.xscale('log')
#pl.title('Dense Matrices', fontsize=24)
#pl.plot(pl.xlim(), pl.ylim(),color='r', ls="--")
#pl.savefig('graphs/dense_scatter.eps',dpi=fig.dpi,bbox_inches='tight')
pl.savefig('scatter.eps',bbox_inches='tight')
pl.show()
