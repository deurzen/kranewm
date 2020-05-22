include config.mk

all: quick_debug

quick_build:
	$(MAKE) -j$$(( 10 * $(shell nproc) )) build

quick_debug:
	$(MAKE) -j$$(( 10 * $(shell nproc) )) debug

debug: CXXFLAGS += $(DEBUG_CXXFLAGS)
debug: LDFLAGS += $(DEBUG_LDFLAGS)
debug: build-core
	+$(MAKE) run tags

build: CXXFLAGS += $(RELEASE_CXXFLAGS)
build: build-core

install:
	install $(BIN)/$(PROJECT) $(DESTDIR)/$(PROJECT)

build-core: notify-build bin obj ${OBJ_FILES} notify-link
	${CC} ${OBJ_FILES} ${LDFLAGS} -o $(BIN)/$(PROJECT)

-include $(DEPS)

obj/%.o: obj

obj/%.o: src/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

obj/%.o: src/x-data/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

run:
	@echo
	@echo running
	@./launch

bin:
	@[ -d bin ] || mkdir bin

obj:
	@[ -d obj ] || mkdir obj

notify-build:
	@echo building

notify-link:
	@echo
	@echo linking

.PHONY: tags
tags:
	@echo
	@echo generating tags
	@ctags -R --exclude=.git --c++-kinds=+p --fields=+iaS --extras=+q .

.PHONY: clean
clean:
	@echo cleaning
	@rm -rf ./bin ./obj
