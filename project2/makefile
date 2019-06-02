CC = gcc
JC = javac

CFLAGS = -g -Wall -Wextra 
CFLAGS2 = -g -Wall -pthread

.SUFFIXES: .java .class
.java.class:
	$(JC) $*.java
JAVA_SRC = \
				ftclient.java \
				ReceiveResponse.java \
				ConnectSocket.java

default: classes ftserver

classes: $(JAVA_SRC:.java=.class)

ftserver : ftserver.c
	$(CC) $(CFLAGS2) -o $@ $^

memcheck : 
	valgrind --leak-check=yes --show-reachable=yes ./ftserver

clean :
	-if [ -f ftserver.o ]; then rm ftserver.o 2> /dev/null; fi
	-if [ -f ftserver ]; then rm ftserver 2> /dev/null; fi
	-rm *.class



