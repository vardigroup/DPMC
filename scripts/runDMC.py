import sys,os

cnf = sys.argv[1]
pw = 0
dmc = ''
if len(sys.argv)>2:
	pw = int(sys.argv[2])
if len(sys.argv)>3:
	dmc = sys.argv[3]
assert('DPMC' in os.environ)

w = 0.5
if len(sys.argv) > 4:
	w_check = float(sys.argv[4])
	if w_check > 0:
		w = w_check

'''
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
'''

#cmd1 = os.environ['DPSAMPLER']+'/lg/build/lg "lg/solvers/flow-cutter-pace17/flow_cutter_pace17 -s 1234567 -p 100" < '+cnf+' > tree.tmp'
#cmd2 = os.environ['DPSAMPLER']+' DMC/dmc --cf='+cnf+' --cs='+cs+' --sf='+sf+'< tree.tmp'
cmd3 = os.environ['DPMC']+'htb/htb --cf='+cnf+' --cv=7 | '+os.environ['DPMC']+'dmc/dmc'+dmc+' --pw='+str(pw)+' --cf='+cnf+' --dp=c --dv=7 --lc=1 --wc=1 --vs=2 --pc=1 --dy=1'
#cmd3 = os.environ['DPMC']+'lg/build/lg "'+os.environ['DPMC']+'lg/solvers/flow-cutter-pace17/flow_cutter_pace17 -s 1234567 -p 100" < '+cnf+' | '+os.environ['DPMC']+'dmc/dmc'+dmc+' --pw='+str(pw)+' --cf='+cnf+' --dp=c --lc=1 --wc=1 --vs=2 --pc=1 --dy=1'
cmd4 = os.environ['DPMC']+'lg/build/lg "'+os.environ['DPMC']+'lg/solvers/htd-master/bin/bin/htd_main -s 1234567 --type hypertree --output human --print-progress --preprocessing full" < '+cnf+' | '+os.environ['DPMC']+'dmc/dmc'+dmc+' --pw='+str(pw)+' --cf='+cnf+' --lc=1 --wc=1 --vs=2'
cmd5 = os.environ['DPMC']+'lg/build/lg "'+os.environ['DPMC']+'lg/solvers/htd-master/bin/bin/htd_main -s 1234567 --print-progress --strategy challenge  --opt width --iterations 0  --preprocessing full" < '+cnf+' | '+os.environ['DPMC']+'dmc/dmc'+dmc+' --pw='+str(pw)+' --cf='+cnf+' --lc=1 --wc=1 --vs=2'
cmd6 = os.environ['DPMC']+'lg/build/lg "'+'/home/adi/Downloads/dmlongo_pace2019/detkdecomp" h < '+cnf+' | '+os.environ['DPMC']+'dmc/dmc'+dmc+' --pw='+str(pw)+' --cf='+cnf+' --lc=1 --wc=1 --vs=2'
cmd7 = os.environ['DPMC']+'lg/build/lg "'+'/home/adi/Downloads/dmlongo_pace2019/detkdecomp" H < '+cnf 
cmd8 = os.environ['DPMC']+'lg/build/lg "python3 infCat.py tests/track1_006_proj.tr" < '+cnf+' | '+os.environ['DPMC']+'dmc/dmc_original'+dmc+' --pw='+str(pw)+' --cf='+cnf+' --dp=c --lc=1 --wc=0 --vs=2 --pc=1'
#print cmd7
#os.system(cmd7)
print (cmd3)
os.system(cmd3)
'''
$DPSAMPLER/lg/build/lg "$DPSAMPLER/lg/solvers/flow-cutter-pace17/flow_cutter_pace17 -s 1234567 -p 100" < test.cnf | $DPSAMPLER/dmc/dmc --pw 0 --cf test.cnf --cs c
$DPSAMPLER/lg/build/lg "$DPSAMPLER/lg/solvers/flow-cutter-pace17/flow_cutter_pace17 -s 1234567 -p 100" < converted_benchmarks/enc1/avgdeg_3_016_1.txt.cnf | $DPSAMPLER/dmc/dmc --pw 0 --cf converted_benchmarks/enc1/avgdeg_3_016_1.txt.cnf --cs c --jp a --dp s --lc 0 --tc 0 
'''
