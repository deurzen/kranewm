include config.mk

all: quick_debug

quick_release:
	$(MAKE) -j$$(( 10 * $(shell nproc) )) release

quick_debug:
	$(MAKE) -j$$(( 10 * $(shell nproc) )) debug

debug: CXXFLAGS += $(DEBUG_CXXFLAGS)
debug: LDFLAGS += $(DEBUG_LDFLAGS)
debug: winsys
debug: core
debug: client
debug: bar
	+$(MAKE) run tags

release: CXXFLAGS += $(RELEASE_CXXFLAGS)
release: LDFLAGS += $(RELEASE_LDFLAGS)
release: winsys
release: core
release: client
release: bar

install:
	install -m0755 $(BINDIR)/$(PROJECT) $(DESTDIR)$(INSTALLDIR)/$(PROJECT)
	install -m0755 $(BINDIR)/$(CLIENT) $(DESTDIR)$(INSTALLDIR)/$(CLIENT)
	install -m0755 $(BINDIR)/$(BAR) $(DESTDIR)$(INSTALLDIR)/$(BAR)

winsys: bin obj ${WINSYS_LINK_FILES}

core: bin obj ${CORE_LINK_FILES}
	${CC} ${CXXFLAGS} ${CORE_LINK_FILES} ${LDFLAGS} -o $(BINDIR)/$(PROJECT)

client: bin obj ${CLIENT_LINK_FILES}
	${CC} ${CXXFLAGS} ${CLIENT_LINK_FILES} ${LDFLAGS} -o $(BINDIR)/$(CLIENT)

bar: bin obj ${BAR_LINK_FILES}
	${CC} ${CXXFLAGS} ${BAR_LINK_FILES} ${LDFLAGS} -o $(BINDIR)/$(BAR)

-include $(DEPS)

obj/%.o: obj

obj/winsys/%.o: src/winsys/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

obj/winsys/xdata/%.o: src/winsys/xdata/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

obj/core/%.o: src/core/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

obj/client/%.o: src/client/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

obj/bar/%.o: src/bar/%.cc
	${CC} ${CXXFLAGS} -MMD -c $< -o $@

run:
	@echo [running]
	@./launch

bin:
	@[ -d bin ] || mkdir bin

obj:
	@[ -d obj ] || mkdir -p obj/{winsys/xdata,core,client,bar}

notify-core:
	@echo [building core]

notify-client:
	@echo [building client]

notify-bar:
	@echo [building bar]

notify-link:
	@echo [linking]

.PHONY: tags
tags:
	@echo [generating tags]
	@git ls-files | ctags -R --exclude=.git --c++-kinds=+p --links=no --fields=+iaS --extras=+q -L-

.PHONY: clean
clean:
	@echo [cleaning]
	@rm -rf ./bin ./obj
