DRB_PATH=Dropbox
MEM_PATH=memStream

all: 
	cd $(MEM_PATH) && $(MAKE) install
	cd $(DRB_PATH) && $(MAKE) install

example: install
	cd $(DRB_PATH) && $(MAKE) example

.PHONY install: all

uninstall: 
	cd $(MEM_PATH) && $(MAKE) uninstall
	cd $(DRB_PATH) && $(MAKE) uninstall

clean:
	cd $(MEM_PATH) && $(MAKE) clean
	cd $(DRB_PATH) && $(MAKE) clean

rebuild: clean all
