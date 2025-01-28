# ----------------------------------------------------------------
#  makefile example for Delta 
#
#  - there is some OS version checking, not really required if you know
#  the OS version, but it would be more work for me to take it out
#  than to leave it in
#
#  - Jim Li, 8Sept2017
# ----------------------------------------------------------------
#
#
#
OS_NAME = $(shell uname -s)
OS_VER  = $(shell uname -r)

ifeq ($(OS_NAME), HP-UX)
   ifeq ($(OS_VER), B.10.20)
      CXX = CC
      CC  = cc

      CXXFLAGS += +DAportable -Aa +z
      CFLAGS   += +DAportable -Ae +z
   endif

   ifeq ($(OS_VER), B.11.11)
      CXX = g++
      CC  = gcc

      CXXFLAGS += -fpic
      CFLAGS   += -fpic
   endif

   ifeq ($(OS_VER), B.11.23)
      CXX = aCC
      CC  = c99

      CXXFLAGS += -D_HPUX_SOURCE -AA
      CFLAGS   += -D_HPUX_SOURCE
   endif

   LDFLAGS += -b

endif

ifeq ($(OS_NAME), Linux)
     CC  = g++
     LD  = g++

     #CXXFLAGS += -m32 -Wno-write-strings -fpic
     CXXFLAGS += -Wall -fpic
     CFLAGS   += -fpic

     CPPFLAGS += -DLINUX -I/usr/include/X11 -I/usr/include

     LDFLAGS += -ldl -lUeiDaq

endif

BINEXT = bin
DLLEXT = so

TARGET = uei_eec_ceres.$(BINEXT)

USEROBJS =custom_init.o ARINC429Scheduler.o

CXXFLAGS += -g
CFLAGS   += -g

#CPPFLAGS += -I$(CERESDIR)/include -I/usr/include/X11R5 -I/usr/include/Motif1.2


all:	$(TARGET)


$(TARGET):	$(USEROBJS)
	@echo "Linking $(TARGET)"
	$(LD) $(LDFLAGS) $^ -o $@
	@echo "Done"


.PHONY:	clean
clean:
	$(RM) *.o eec_uei.$(BINEXT)


%: %.C
#  commands to execute (built-in):
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

