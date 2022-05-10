dmc:../addmc/src/implementation/* ../addmc/src/interface/* ../addmc/lib.zip ../addmc/Makefile
	make --directory=../addmc clean && \
	make --directory=../addmc dmc OPT=-Ofast && \
	cp ../addmc/dmc .

dmc.sif:../addmc/src/implementation/* ../addmc/src/interface/* ../addmc/lib.zip ../addmc/Makefile Singularity
	singularity build dmc.sif Singularity

clean:
	rm -f dmc dmc.sif
