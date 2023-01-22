OBJS = serial.o semaphore.o screen.o emergency.o

main: main.c $(OBJS)
	gcc -o main $(OBJS) main.c -pthread -Wall -lncursesw

serial.o: serial.c serial.h
	gcc -c serial.c -Wall

semaphore.o: semaphore.c semaphore.h
	gcc -c semaphore.c -Wall

screen.o: screen.c screen.h
	gcc -c screen.c -Wall

emergency.o: emergency.c emergency.h
	gcc -c emergency.c -Wall

clean:
	rm *.o
	rm main