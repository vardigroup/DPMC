dmc:../common/src/implementation/* ../common/src/interface/* ../common/lib.zip ../common/Makefile
	make --directory=../common clean && \
	make --directory=../common dmc OPT=-Ofast && \
	cp ../common/dmc .

dmc.sif:../common/src/implementation/* ../common/src/interface/* ../common/lib.zip ../common/Makefile Singularity
	singularity build dmc.sif Singularity

clean:
	rm -f dmc dmc.sif
