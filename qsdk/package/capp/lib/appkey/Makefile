
OBJS=appkey.o
TARGET=libappkey.so
LIBS_DEPEND=-lubacktrace

.PHONY:all
all:$(TARGET)
	
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

$(TARGET):$(OBJS)
	echo "OS_TYPE=$(OS_TYPE)"
	${CC} ${CFLAGS} ${LDFLAGS} $(LIBS_DEPEND) -o $(TARGET) $(OBJS)
%.o:%.c
	${CC} -c ${CFLAGS} $< -o $@