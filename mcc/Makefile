DMC_TARGET = bin/dmc
LG_TARGET = bin/lg
FLOW_TARGET = bin/flow_cutter_pace17
TARGETS = $(DMC_TARGET) $(LG_TARGET) $(FLOW_TARGET)

BIN_FILES = bin/driver.py bin/pmc $(TARGETS)
DOCS = README.md starexec_description.txt

ZIP0 = dpmc.zip
FILES0 = bin/starexec_run_* $(BIN_FILES) $(DOCS)
$(ZIP0): $(FILES0)
	zip $(ZIP0) -FS $(FILES0)

ZIP1 = dpmc1.zip
FILES1 = bin/starexec_run_1pre1 $(BIN_FILES) $(DOCS)
$(ZIP1): $(FILES1)
	zip $(ZIP1) -FS $(FILES1)

ZIP2 = dpmc2.zip
FILES2 = bin/starexec_run_2pre1 $(BIN_FILES) $(DOCS)
$(ZIP2): $(FILES2)
	zip $(ZIP2) -FS $(FILES2)

ZIP3 = dpmc3.zip
FILES3 = bin/starexec_run_3pre1 $(BIN_FILES) $(DOCS)
$(ZIP3): $(FILES3)
	zip $(ZIP3) -FS $(FILES3)

ZIP4 = dpmc4.zip
FILES4 = bin/starexec_run_4pre1 $(BIN_FILES) $(DOCS)
$(ZIP4): $(FILES4)
	zip $(ZIP4) -FS $(FILES4)

$(DMC_TARGET): ../addmc/src/*
	make -C ../addmc dmc opt=-Ofast link=-static
	ln -sf ../../addmc/dmc bin

$(LG_TARGET): ../lg/src/*
	make -C ../lg build/lg
	ln -sf ../../lg/build/lg bin

$(FLOW_TARGET): ../lg/solvers/flow-cutter-pace17/src/*
	make -C ../lg/solvers/flow-cutter-pace17 flow_cutter_pace17
	ln -sf ../../lg/solvers/flow-cutter-pace17/flow_cutter_pace17 bin

.PHONY: all clean

all: $(ZIP0) $(ZIP1) $(ZIP2) $(ZIP3) $(ZIP4)

clean:
	rm -f $(TARGETS) *.zip
