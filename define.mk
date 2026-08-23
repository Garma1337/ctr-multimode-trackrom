DECOMP_NOWARN := -Wno-int-conversion \
                 -Wno-incompatible-pointer-types \
                 -Wno-implicit-function-declaration \
                 -Wno-implicit-fallthrough \
                 -Wno-discarded-qualifiers

src/hooks/exe/load_stages.o: CPPFLAGS += $(DECOMP_NOWARN)
