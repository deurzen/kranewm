include config.mk

all: build

quick_build:
	$(MAKE) -j$$(( 10 * $(shell nproc) )) build

quick_debug:
	$(MAKE) -j$$(( 10 * $(shell nproc) )) debug

debug: CXXFLAGS += $(DEBUG_CXXFLAGS)
debug: LDFLAGS += $(DEBUG_LDFLAGS)
debug: build-core
debug: build-client
	+$(MAKE) run tags

build: CXXFLAGS += $(RELEASE_CXXFLAGS)
build: LDFLAGS += $(RELEASE_LDFLAGS)
build: build-core
build: build-client

install:
	install $(BIN)/$(PROJECT) $(DESTDIR)/$(PROJECT)
	install $(BIN)/$(CLIENT) $(DESTDIR)/$(CLIENT)

build-core: bin obj ${OBJ_FILES}
	${CC} ${OBJ_FILES} ${LDFLAGS} -o $(BIN)/$(PROJECT)

build-client: bin obj ${KRANEC_OBJ_FILES}
	${CC} ${KRANEC_OBJ_FILES} ${LDFLAGS} -o $(BIN)/$(CLIENT)

-include $(DEPS)

obj/%.o: obj

obj/%.o: src/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

obj/x-data/%.o: src/x-data/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

obj/kranec/%.o: src/kranec/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

run:
	@echo
	@echo running
	@./launch

bin:
	@[ -d bin ] || mkdir bin

obj:
	@[ -d obj ] || mkdir -p obj/{x-data,kranec}

notify-build:
	@echo building kranewm

notify-client:
	@echo building kranec

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
