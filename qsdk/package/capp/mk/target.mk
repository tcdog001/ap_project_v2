.PHONY: all
all:$(TARGET) install



$(TARGET):$(OBJS)
	$(dump_common_vars)
	${CC} ${CFLAGS} ${__CFLAGS} ${LDFLAGS} ${__LDFLAGS} -o $(TARGET) $(OBJS) $(lib_depend)
	@if [ -f "$(FILENO_NAME).key" ]; then \
		mkdir -p $(APPKEY_PATH); \
		cp -f $(FILENO_NAME).key $(APPKEY_PATH); \
	fi
%.o:%.c
	${CC} -c ${CFLAGS} ${__CFLAGS} $(INCS) $< -o $@



pre:$(DEPS) $(PRES)
	$(dump_common_vars)
%.d:%.c
	${CC} -MMD -E $(<) ${CFLAGS} ${__CFLAGS} $(INCS) >/dev/null
%.i:%.c
	${CC} -E $(<) ${CFLAGS} ${__CFLAGS} $(INCS) >>$(@)


.PHONY: check
check:
	$(do_check)



.PHONY: clean
clean:
	$(do_clean)



.PHONY: install
install:
	$(do_install)
