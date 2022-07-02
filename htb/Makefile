htb: ../addmc/src/*
	make -C ../addmc clean-htb
	make -C ../addmc htb opt=-Ofast link=-static
	rm -f htb
	cp ../addmc/htb .

.PHONY: clean

clean:
	rm -f htb
