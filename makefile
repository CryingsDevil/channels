readingroom: readingroom.o
	gcc -std=gnu99 -pthread -lpthread -o readingroom readingroom.o

readingroom.o: readingroom.c 
	gcc -std=gnu99 -pthread -lpthread -c readingroom.c 

clean:
	rm readingroom readingroom.o
