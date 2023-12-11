CFLAGS =	-O0 -g -ggdb -Wall -Werror -fmessage-length=0 -fshort-enums

OBJS =		main.o vm_exe.o

LIBS = -lncurses

#CFLAGS += -I../shared

TARGET =	vm

$(TARGET):	$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
