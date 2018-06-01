TARGETS := $(MAKECMDGOALS)

.PHONY: all $(TARGETS)
all: $(TARGETS)

$(TARGETS):
	mksys -v -t $@ \
		trafficLight.sdef
	systoimg $@ trafficLight.$@.update _build_trafficLight.$@ || true
	[ ! -e "_build_trafficLight.$@/legato.cwe" ] || cp "_build_trafficLight.$@/legato.cwe" trafficLight.$@.cwe

clean:
	rm -rf _build_* *.update
