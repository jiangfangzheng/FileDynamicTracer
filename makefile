#MAKE FILE 

RM 	= rm 
CC 	= gcc
SYSROOT = 

PIC	= -c -fPIC 
SHARED	= -shared
DL	= -ldl
OPTION	= -Wl,--no-as-needed #-m32 

FILE 	= JfzTracer
default:
	$(CC) $(SYSROOT) $(PIC) $(FILE).c 
	$(CC) $(SYSROOT) $(OPTION) $(SHARED) $(FILE).o -o lib$(FILE).so $(DL)

clean:
	$(RM) lib$(FILE).so $(FILE).o
