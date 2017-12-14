
STRIP			= strip
#DEBUG		= -g3
OPTIMIZE = -O3 -Wall
#STATIC		= -static
DEFINES 	= -D__USE_REENTRANT -D_REENTRANT -D_THREAD_SAFE -D_LIBC_REENTRANT -D_GNU_SOURCE
INCLUDES	= -I.
CFLAGS		= $(DEFINES) $(INCLUDES) $(DEBUG) $(OPTIMIZE)
CPPFLAGS	= $(CFLAGS) -std=gnu++11
LIBS		+= -lssl -lcrypto
LDFLAGS		= $(STATIC) $(DEBUG) -Wl,-Bdynamic -Wl,-O6 -Wl,--start-group $(LIBS) -Wl,--end-group
ENC_TARGET		= enc
DEC_TARGET		= dec

ENC_OBJS = enc.o
DEC_OBJS = dec.o

all:	do-it-all

ifeq (.depend,$(wildcard .depend))
include .depend
do-it-all:	$(ENC_TARGET) $(DEC_TARGET)
else
do-it-all:	depend $(ENC_TARGET) $(DEC_TARGET)
endif

$(ENC_TARGET):	$(ENC_OBJS)
	$(CXX) $(ENC_OBJS) -o $(ENC_TARGET) $(LDFLAGS)
	$(STRIP) -s $(ENC_TARGET)

$(DEC_TARGET):	$(DEC_OBJS)
	$(CXX) $(DEC_OBJS) -o $(DEC_TARGET) $(LDFLAGS)
	$(STRIP) -s $(DEC_TARGET)

.cpp.o:
	$(CXX) $(CPPFLAGS) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f *.bin test.* DEADJOE .depend *.o *~ *core $(ENC_TARGET) $(DEC_TARGET) nohup.out && echo Clean Ok.
                
dep depend:
	$(CC) -M *.cpp > .depend

check: $(ENC_TARGET) $(DEC_TARGET)
	@./check.sh

dist:	dep clean all
