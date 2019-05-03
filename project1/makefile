CC = gcc
JC = javac

CFLAGS = -g -Wall -Wextra 
CFLAGS2 = -g -Wall

.SUFFIXES: .java .class
.java.class:
	$(JC) $*.java
JAVA_SRC = \
				chatserve.java \
				SpeakingThread.java

default: classes chatclient

classes: $(JAVA_SRC:.java=.class)

chatclient : chatclient.c
	$(CC) $(CFLAGS2) -o $@ $^

memcheck : 
	valgrind --leak-check=yes --show-reachable=yes ./chatclient

clean :
	-if [ -f chatclient.o ]; then rm chatclient.o 2> /dev/null; fi
	-if [ -f chatclient ]; then rm chatclient 2> /dev/null; fi
	-if [ -f chatserve.class ]; then rm chatserve.class 2> /dev/null; fi
	-if [ -f SpeakingThread.class ]; then rm SpeakingThread.class 2> /dev/null; fi
	-if [ -f EchoClient.class ]; then rm EchoClient.class 2> /dev/null; fi
