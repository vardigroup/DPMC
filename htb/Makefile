htb:../addmc/src/implementation/* ../addmc/src/interface/* ../addmc/lib.zip ../addmc/Makefile
	make --directory=../addmc clean && \
	make --directory=../addmc htb OPT=-Ofast && \
	cp ../addmc/htb .

htb.sif:../addmc/src/implementation/* ../addmc/src/interface/* ../addmc/lib.zip ../addmc/Makefile Singularity
	singularity build htb.sif Singularity

clean:
	rm -f htb htb.sif
