salida_out = padre
child_process = salida2


salida_headers = 
salida_source  = $(salida_headers:.h=.c) forkExecPipe.c
salida_objects = $(salida_source:.c=.o)

gray_headers = 
gray_process_source = $(gray_headers:.h=.c) hijoForkExecPipe.c
gray_process_objects = $(gray_process_source:.c=.o)

CC     = gcc
CFLAGS = -Wall

depends = .depends

build : $(salida_out) 
build : $(child_process)

$(salida_out) : $(salida_objects)
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(child_process) : $(gray_process_objects)
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(objects) :
	$(CC) $(CFLAGS) -c -o $@ $*.c

$(depends) : $(salida_source) $(salida_headers)
	@$(CC) -MM $(salida_source) > $@


clean :
	$(RM) $(salida_out) $(salida_objects) $(zipfile) $(depends)

.PHONY : build zip clean

sinclude $(depends)
