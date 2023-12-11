CFLAGS =	-O0 -g -ggdb -Wall -Werror -fmessage-length=0 -fshort-enums

OBJS =		main.o parser_gen.o parser.o parse_exp.o parser_reg.o vm_funcs.o vm_code.o ../shared/vector.o

LIBS =

#CFLAGS += -I../shared

TARGET =	compss

$(TARGET):	$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
