CXX = mpicxx
LIBHEAR_CXX_FLAGS = -shared -fPIC

LIBHEAR_OBJS = hear.po

%.po: %.cpp
	$(CXX) $(LIBHEAR_CXX_FLAGS) -o $@ -c $<

libhear.so : $(LIBHEAR_OBJS)
	$(CXX) $(LIBHEAR_CXX_FLAGS) -o $@ $(LIBHEAR_OBJS)

. : libhear.so

clean:
	rm -rf *.po src/*.po *.so
