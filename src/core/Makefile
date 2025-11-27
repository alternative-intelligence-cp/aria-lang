CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -fPIC `llvm-config --cflags`
LDFLAGS = -lm `llvm-config --ldflags --libs core mc target analysis bitwriter transforms`

SOURCES = tesla_consciousness_scheduler.c tesla_runtime.c tesla_memory.c llvm_integration.c tesla_debug_info.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = libtesla_core.a
SHARED_TARGET = libtesla_core.so

.PHONY: all clean install

all: $(TARGET) $(SHARED_TARGET)

$(TARGET): $(OBJECTS)
	ar rcs $(TARGET) $(OBJECTS)

$(SHARED_TARGET): $(OBJECTS)
	$(CC) -shared -o $(SHARED_TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) $(SHARED_TARGET)

install: all
	mkdir -p ../lib
	cp $(TARGET) $(SHARED_TARGET) ../lib/
	mkdir -p ../include
	cp *.h ../include/

# Dependencies
tesla_consciousness_scheduler.o: tesla_consciousness_scheduler.h
tesla_runtime.o: tesla_runtime.h tesla_consciousness_scheduler.h
tesla_memory.o: tesla_memory.h
llvm_integration.o: llvm_integration.h
tesla_debug_info.o: tesla_debug_info.h llvm_integration.h