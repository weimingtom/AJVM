
include config.mk


CFILES 	:= $(shell find $(TOP)/src -type f -name "*.c")
CXXFILES:= $(shell find $(TOP)/src -type f -name "*.cpp")
AFILES	:= $(shell find $(TOP)/src -type f -name "*.s")
HFILES	:= $(shell find $(TOP)/src -type f -name "*.h")

SFILES 	:= $(CFILES) $(CXXFILES) $(AFILES)
OFILES	:= $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) $(AFILES:.s=.o)


PACKAGE	:= libjvm
VERSION	:= 0.1

OUTPUT	:= $(PACKAGE).a
TESTOUT	:= jvm

.PHONY: all clean git

all: $(TESTOUT)
$(TESTOUT): $(OUTPUT)
	@echo "  LD      " $@
	@$(LD) -o $@ $(OFILES)

$(OUTPUT): $(OFILES)
	@echo "  AR      " $@
	@$(AR) -rcs $@ $(OFILES)

.c.o:
	@echo "  CC      " $(notdir $<)
	@$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	@echo "  CXX     " $(notdir $<)
	@$(CXX) $(CXXFLAGS) $< -o $@

.s.o:
	@echo "  ASM     " $(notdir $<)
	@$(ASM) $(AFLAGS) $< -o $@
	

clean:
	-@$(RM) $(OUTPUT)
	-@$(RM) $(OFILES)

distclean: clean
	-@$(RM) $(PACKAGE)-$(VERSION)-$(ARCH).tar.gz

dist: install
	-@mv bin $(PACKAGE)-$(VERSION)-$(ARCH)
	-@tar -czf $(PACKAGE)-$(VERSION)-$(ARCH).tar.gz $(PACKAGE)-$(VERSION)-$(ARCH)/*
	-@mv $(PACKAGE)-$(VERSION)-$(ARCH) bin

git: clean
	-@git add --all .
	-@git commit -m "$(COMMIT)"
	-@git push origin master
	-@$(RM) -r usr
	
	
