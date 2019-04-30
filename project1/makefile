CC = gcc
JAVA = javac
CFLAGS = -g -Wall -Wextra 
CFLAGS2 = -g -Wall

chatclient : chatclient.c
	$(CC) $(CFLAGS2) -o $@ $^

chatserve : chatserve.java
	$(JAVA) $@ $^

echo : EchoClient.java
	$(JAVA) $@ $^

memcheck : 
	valgrind --leak-check=yes --show-reachable=yes ./chatclient

clean :
	-if [ -f chatclient.o ]; then rm chatclient.o 2> /dev/null; fi
	-if [ -f chatclient ]; then rm chatclient 2> /dev/null; fi
	-if [ -f chatserve.class ]; then rm chatserve.class 2> /dev/null; fi
	-if [ -f SpeakingThread.class ]; then rm SpeakingThread.class 2> /dev/null; fi
	-if [ -f EchoClient.class ]; then rm EchoClient.class 2> /dev/null; fi
