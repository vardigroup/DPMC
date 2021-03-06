htb:../common/src/implementation/* ../common/src/interface/* ../common/lib.zip ../common/Makefile
	make --directory=../common clean && \
	make --directory=../common htb OPT=-Ofast && \
	cp ../common/htb .

htb.sif:../common/src/implementation/* ../common/src/interface/* ../common/lib.zip ../common/Makefile Singularity
	singularity build htb.sif Singularity

clean:
	rm -f htb htb.sif
