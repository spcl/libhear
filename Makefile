CXX = mpicxx
LIBHEAR_CXX_FLAGS = -shared -fPIC

LIBHEAR_OBJS = hear.po

%.po: %.cpp
	$(CXX) $(LIBHEAR_CXX_FLAGS) -o $@ -c $<

libhear.so : LIBHEAR_CXX_FLAGS += -O3
libhear.so : $(LIBHEAR_OBJS)
	$(CXX) $(LIBHEAR_CXX_FLAGS) -o $@ $(LIBHEAR_OBJS)

. : libhear.so

libhear_debug.so : LIBHEAR_CXX_FLAGS += -g -O0 -D DEBUG=1
libhear_debug.so : $(LIBHEAR_OBJS)
	$(CXX) $(LIBHEAR_CXX_FLAGS) -o $@ $(LIBHEAR_OBJS)

debug: libhear_debug.so

clean:
	rm -rf *.po src/*.po *.so
