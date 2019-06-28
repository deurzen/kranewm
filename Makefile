include config.mk

all: debug

quick_all:
	$(MAKE) -j39 all

quick_build:
	$(MAKE) -j39 build

debug: quick_debug

quick_debug:
	$(MAKE) -j39 debug_parallel

debug_parallel: TARGET = $(BIN)
debug_parallel: CXXFLAGS += $(DEBUG_CXXFLAGS)
debug_parallel: LDFLAGS += $(DEBUG_LDFLAGS)
debug_parallel: build
	@echo
	@echo -n running
	@./launch
	@echo
	@echo generating tags
	@ctags -R --exclude=.git --c++-kinds=+p --fields=+iaS --extras=+q .

install:
	install $(RELEASE) $(INSTALL)$(PROJECT)

release:
	@[ -d release ] || mkdir release


include test/config.mk

test: test/bin test/obj ${TEST_OBJ_FILES}
	${CC} ${CXXFLAGS} ${TEST_OBJ_FILES} ${LDFLAGS} -o ${TESTTARGET}
	@echo
	@echo running tests
	@echo
	@./test/bin/tests

test/obj/%.o: test/obj

test/obj/%.o: test/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

test/run:
	@echo running tests
	@echo
	@./test/bin/tests

-include test/config.mk


bin:
	@[ -d bin ] || mkdir bin

test/bin:
	@[ -d test/bin ] || mkdir test/bin

obj:
	@[ -d obj ] || mkdir obj

test/obj:
	@[ -d test/obj ] || mkdir test/obj

notify-build:
	@echo building

notify-link:
	@echo
	@echo linking

build: notify-build release bin obj ${OBJ_FILES} notify-link
	${CC} ${CXXFLAGS} ${OBJ_FILES} ${LDFLAGS} -o ${TARGET}

-include $(DEPS)

obj/%.o: obj

obj/%.o: src/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

obj/%.o: src/x-data/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

run:
	@echo
	@echo -n running
	@./launch

.PHONY: tags
tags:
	@echo
	@echo generating tags
	@ctags -R --exclude=.git --c++-kinds=+p --fields=+iaS --extras=+q .

.PHONY: clean
clean:
	@echo cleaning
	@rm -rf ./bin ./release ./obj

.PHONY: cleantest
cleantest:
	@echo cleaning tests
	@rm -rf ./test/bin ./test/obj
