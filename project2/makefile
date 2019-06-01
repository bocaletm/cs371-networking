CC = gcc
JC = javac

CFLAGS = -g -Wall -Wextra 
CFLAGS2 = -g -Wall -pthread

#.SUFFIXES: .java .class
#.java.class:
#	$(JC) $*.java
#JAVA_SRC = \
				chatserve.java \
				SpeakingThread.java \
				Globals.java

#default: classes ftserver

default: ftserver

#classes: $(JAVA_SRC:.java=.class)

ftserver : ftserver.c
	$(CC) $(CFLAGS2) -o $@ $^

memcheck : 
	valgrind --leak-check=yes --show-reachable=yes ./chatclient

clean :
	-if [ -f ftserver.o ]; then rm ftserver.o 2> /dev/null; fi
	-if [ -f ftserver ]; then rm ftserver 2> /dev/null; fi
