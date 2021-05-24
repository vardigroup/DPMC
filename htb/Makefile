htb: ../addmc/src/*
	make -C ../addmc htb opt=-Ofast link=-static
	ln -sf ../addmc/htb .

.PHONY: clean

clean:
	rm -f htb
