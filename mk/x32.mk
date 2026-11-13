# Native pTOS ILP32 programs using the shared CRT and AES library.
# PTOS provides the shared runtime and caller-owned GEMlib AES bindings.
PTOS ?= ../pTOS
X32_PREFIX ?=
X32_CC ?= $(X32_PREFIX)gcc
X32_OBJCOPY ?= $(X32_PREFIX)objcopy
X32_CFLAGS = $(STD) $(WARN) -Os -mx32 -ffreestanding \
    -fPIC -fvisibility=hidden -fno-stack-protector -fno-builtin \
    -Isrc/compat -Isrc -Ibuild -I$(GEMLIB) -I$(LIBCMINI)/include -MMD -MP
X32_OBJ = $(patsubst src/%.c,build/x32/%.o,$(LOGIC_SRC) $(GEM_SRC) $(GENERIC_PLATFORM_SRC))
X32_LIBCMINI = build/x32/lib/libcmini.a
X32_VDI_NAMES = rc_copy rc_grect_to_array v_bar v_clsvwk v_opnvwk v_pline \
    vq_extnd vqt_attributes vr_recfl vro_cpyfm vrt_cpyfm vs_clip vsf_color \
    vsf_interior vsf_style vsf_perimeter vsf_udpat vsl_color vsl_type vsl_width \
    vst_color vst_height vswr_mode vdi_dummy v_gtext v_gtext16n v_hide_c v_show_c \
    vq_mouse vqt_extent vqt_extent16n vqt_fontinfo vqt_name vqt_width \
    vst_alignment vst_font vst_point vst_load_fonts vst_unload_fonts \
    n_vqt_ext_name n_vqt_xfntinfo n_vst_arbpt n_vst_map_mode \
    vsl_ends vst_effects vsm_height vdi_str2array vdi_array2str
X32_VDI_OBJ = $(addprefix build/x32/gemlib/,$(addsuffix .o,$(X32_VDI_NAMES)))
X32_OBJ += build/x32/platform/x32/vdi.o build/x32/platform/x32/events.o
build/x32/platform/x32/vdi.o: X32_CFLAGS += -I$(PTOS)/include -I$(PTOS)/obj

# Use GEMlib's portable C parameter marshalling, calling the native VDI
# entry through src/platform/x32/vdi.c instead of its m68k trap binding.
# Rejected native queries leave GEMlib output arrays untouched, so zero them.
build/x32/gemlib/%.o: $(GEMLIB)/%.c $(GEMLIB_H)
	@mkdir -p $(dir $@)
	$(X32_CC) $(X32_CFLAGS) -ftrivial-auto-var-init=zero -D__NO_INLINE__ -c $< -o $@

build/x32/lib/libptos-vdi.a: $(PTOS)/obj/x32/libptos-vdi.a
	@mkdir -p $(dir $@)
	$(X32_OBJCOPY) --redefine-sym vdi=typeshelf_native_vdi $< $@

.PHONY: x32
x32: build/x32/TYPESHLF.PRG build/x32/TYPESHLF.RSC

build/x32/%.o: src/%.c $(GEMLIB_H) $(VERSION_H)
	@mkdir -p $(dir $@)
	$(X32_CC) $(X32_CFLAGS) -c $< -o $@

$(X32_LIBCMINI): $(LIBCMINI_REV)
	rm -rf build/x32/libcmini
	mkdir -p build/x32/libcmini $(dir $@)
	$(LIBCMINI_COPY) build/x32/libcmini/
	$(MAKE) -C build/x32/libcmini CROSSPREFIX=$(X32_PREFIX) CC='$(X32_CC)' ptos_x86_64_defconfig
	$(MAKE) -C build/x32/libcmini CROSSPREFIX=$(X32_PREFIX) CC='$(X32_CC)' \
	    ARCH_CFLAGS='-fPIC -fvisibility=hidden -fno-stack-protector' dirs libs
	cp build/x32/libcmini/build/libcmini.a $@

build/x32/TYPESHLF.PRG: $(X32_OBJ) $(X32_VDI_OBJ) $(X32_LIBCMINI) build/x32/lib/libptos-vdi.a \
    $(PTOS)/obj/ldso/dyncrt.o $(PTOS)/obj/ldso/libptos.so $(PTOS)/obj/libaes/libaes.so \
    $(PTOS)/obj/gemlib/libgem.a
	$(X32_CC) -mx32 -nostdlib -no-pie \
	    -Wl,--hash-style=sysv,-z,now,-z,noexecstack,-e,_start \
	    -Wl,--dynamic-linker,/GEMSYS/LIB/X32/LD.SO,-Ttext-segment=0x400000 \
	    $(PTOS)/obj/ldso/dyncrt.o $(X32_OBJ) $(X32_VDI_OBJ) \
	    $(PTOS)/obj/gemlib/libgem.a \
	    -L$(PTOS)/obj/libaes -laes -L$(PTOS)/obj/ldso -lptos \
	    build/x32/lib/libptos-vdi.a $(X32_LIBCMINI) -o $@

-include $(X32_OBJ:.o=.d) $(X32_VDI_OBJ:.o=.d)

# Run the real adapter with a native-entry mock on the host. Real GEMlib
# headers stay in use; only pTOS's pointer layout is widened for host execution.
.PHONY: test-x32-adapter
test-x32-adapter: build/host/test-x32-adapter
	./build/host/test-x32-adapter

build/host/test-x32-adapter: tests/x32/test_vdi.c tests/x32/vdi_user.h \
    src/platform/x32/vdi.c src/gdosinfo/decode.h $(GEMLIB_H)
	@mkdir -p $(dir $@)
	$(CC_HOST) $(HOST_FLAGS) -Itests/x32 -Isrc/compat -I$(GEMLIB) \
	    tests/x32/test_vdi.c src/platform/x32/vdi.c -o $@
