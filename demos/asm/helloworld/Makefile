AS=gbas
LD=gbld
OBJ=hellow.o sprites.o
TARGET=hellow.gb

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(OBJ) -o $@

%.o: %.s
	$(AS) -c -o $@ $<

clean:
	rm *.o

