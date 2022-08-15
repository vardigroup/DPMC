dmc: ../addmc/src/* ../addmc/makefile
	make -C ../addmc clean-dmc
	make -C ../addmc dmc opt=-Ofast link=-static
	rm -f dmc
	cp ../addmc/dmc .

dmc.sif: Singularity ../addmc/src/* ../addmc/Makefile
	make -C ../addmc clean-libraries
	singularity build -F dmc.sif Singularity

.PHONY: clean

clean:
	rm -f dmc dmc.sif
