dmc.sif: ../addmc/src/*
	singularity build dmc.sif Singularity

dmc: ../addmc/src/*
	make -C ../addmc dmc opt=-Ofast
	ln -sf ../addmc/dmc .

.PHONY: clean

clean:
	rm -f dmc dmc.sif
