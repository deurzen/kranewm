include config.mk

all: debug

quick_all:
	$(MAKE) -j39 all

debug: quick_debug

quick_debug:
	$(MAKE) -j39 debug_parallel

debug_parallel: TARGET = $(BIN)
debug_parallel: CXXFLAGS += $(DEBUG_FLAGS)
debug_parallel: build
	@echo
	@echo -n running
	@./launch
	@echo
	@echo generating tags
	@ctags -R --exclude=.git --c++-kinds=+p --fields=+iaS --extras=+q .

# TODO
# debug: TARGET = $(BIN)
# debug: CXXFLAGS += $(DEBUG_FLAGS)
# debug: build run tags

install:
	install $(RELEASE) /usr/$(BIN)

release:
	@[ -d release ] || mkdir release

bin:
	@[ -d bin ] || mkdir bin

obj:
	@[ -d obj ] || mkdir obj

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
