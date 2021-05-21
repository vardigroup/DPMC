import sys,os

cnf = sys.argv[1]
cs = sys.argv[2]
ns = 1000
if len(sys.argv) > 3:
	ns = int(sys.argv[3])
w = 0.5
if len(sys.argv) > 4:
	w = float(sys.argv[4])
sf = 'samples.txt'
if len(sys.argv) > 5:
	sf = sys.argv[5]

cnf2 = '/tmp/wtd'+'_wt='+str(w)+'.cnf'
f1 = open(cnf,'r')
f2 = open(cnf2,'w')
addWts = True
for line in f1:
	f2.write(line)
	if line.startswith('p cnf'):
		nVars = int(line.split()[2])
		print 'nVars=',nVars
		wts = ''
		for i in range(2*nVars):
			wts += str(w)+' '
	if line.startswith('c weights'):
		addWts = False
if addWts:
	f2.write('c weights '+wts+'\n')
f1.close()
f2.close()
cnf = cnf2

cmd1 = 'lg/build/lg "lg/solvers/flow-cutter-pace17/flow_cutter_pace17 -s 1234567 -p 100" < '+cnf+' > tree.tmp'
cmd2 = ' DMC/dmc --cf='+cnf+' --jf=tree.tmp --pf=1e-3 --cs='+cs+' --sf='+sf
cmd3 = 'lg/build/lg "lg/solvers/flow-cutter-pace17/flow_cutter_pace17 -s 1234567 -p 100" < '+cnf+' | DMC/dmc --wf=3 --cf='+cnf+' --jf=- --pf=1e-3 --cs='+cs+' --sf='+sf+' --ns='+str(ns)
print cmd3
os.system(cmd3)
#print cmd2
#os.system(cmd2)
