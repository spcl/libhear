CXX = g++
MPICXX = mpicxx
INCLUDE_DIR = $(shell pwd)/include/
SRC_DIR = $(shell pwd)/src/
TESTS_DIR = $(shell pwd)/tests/
DEBUG_FLAGS = -g -O0
RELEASE_FLAGS = -O3 -ffast-math -march=native

LIBHEAR_CXX_FLAGS = -I$(INCLUDE_DIR)
LIBHEAR_OBJS = mpool.po encrypt.po hear.po

%.po: $(SRC_DIR)/%.cpp
	$(MPICXX) $(LIBHEAR_CXX_FLAGS) -fPIC -o $@ -c $<

libhear.so: $(LIBHEAR_OBJS)
	$(MPICXX) $(LIBHEAR_CXX_FLAGS) -fPIC -shared -o $@ $(LIBHEAR_OBJS)

hear_naive : LIBHEAR_CXX_FLAGS += $(RELEASE_FLAGS)
hear_naive : $(LIBHEAR_OBJS) libhear.so

hear_mpool_only : LIBHEAR_CXX_FLAGS += $(RELEASE_FLAGS) -D USE_MPOOL=1
hear_mpool_only : $(LIBHEAR_OBJS) libhear.so

hear_release : LIBHEAR_CXX_FLAGS += $(RELEASE_FLAGS) -D USE_MPOOL=1 -D USE_PIPELINING=1
hear_release :  $(LIBHEAR_OBJS) libhear.so

hear_debug : LIBHEAR_CXX_FLAGS += $(DEBUG_FLAGS) -D DCHECK=1
hear_debug :  $(LIBHEAR_OBJS) libhear.so

encr_perf_test : LIBHEAR_CXX_FLAGS += $(RELEASE_FLAGS)
encr_perf_test : encrypt.po $(TESTS_DIR)/encryption_perf.cpp
	$(CXX) $(LIBHEAR_CXX_FLAGS) -o $@ $(TESTS_DIR)/encryption_perf.cpp encrypt.po

debug: hear_debug

release : hear_release

clean:
	rm -rf *.po src/*.po *.so
