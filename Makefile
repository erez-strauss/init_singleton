
CXXFLAGS:= -mcx16 -std=c++2a -I. -Iexamples -W -Wall -Wextra -Wshadow -Wpedantic -O3 -pthread

BDIR:=build
VPATH:= src:tests:examples:.
GTEST_INCLUDEDIR := $(shell if [ -d /usr/include/gtest ]; then echo /usr/include ; fi )
GTEST_LIBDIR := $(shell if [ -f /usr/lib64/libgtest.so ]; then echo /usr/lib64 ; fi )

TARGETS:= $(BDIR)/singleton1 $(BDIR)/singleton2 $(BDIR)/singleton3 $(BDIR)/singleton4bad $(BDIR)/singleton5 $(BDIR)/singleton6 $(BDIR)/singleton7 $(BDIR)/singleton8 $(BDIR)/singleton9 $(BDIR)/singleton10 $(BDIR)/gtest_singleton1 $(BDIR)/gtest_app_singleton1

ifneq ($(GTEST_INCLUDEDIR),)
	TARGETS += $(BDIR)/gtest_singleton1
endif

LINK.o := $(LINK.cc)

all: $(TARGETS) | $(BDIR)/.f

run_tests: all $(TARGETS) | $(BDIR)/.f
	@for p in $(TARGETS); do echo ===== $$p ===== ; ./$$p ; done

run_valgrind: all $(TARGETS) | $(BDIR)/.f
	@for p in $(TARGETS); do echo ===== $$p ===== ; valgrind -v --leak-check=full --track-origins=yes --log-file=./$$p.vg.out.txt ./$$p ; done

$(BDIR)/singleton3: $(BDIR)/singleton3.o $(BDIR)/singleton3a.o $(BDIR)/singleton3b.o $(BDIR)/singleton3c.o

$(BDIR)/gtest_app_singleton1: LDFLAGS += -lgtest_main -lgtest 
$(BDIR)/gtest_app_singleton1: CXXFLAGS += -lgtest_main -lgtest 

$(BDIR)/gtest_singleton1: LDFLAGS += -lgtest_main -lgtest 
$(BDIR)/gtest_singleton1: CXXFLAGS += -lgtest_main -lgtest 

$(BDIR)/%: $(BDIR)/%.o | $(BDIR)/.f
	$(CXX) $(CXXFLAGS) $(LDFALGS) -o $@ $^ 

$(BDIR)/gtest_app_singleton1:  $(BDIR)/gtest_app_singleton1a.o $(BDIR)/gtest_app_singleton1b.o
	$(CXX) $(CXXFLAGS) $(LDFALGS) -o $@ $^ 

$(BDIR)/%.o: $(BDIR)/.f

$(BDIR)/%.o: %.cpp | $(BDIR)/.f
	$(CXX) $(CXXFLAGS) -I. -Isrc -DGTEST_HAS_PTHREAD=1 -pthread -c -o $@ $^

cmake:
	mkdir cbuild; cd cbuild ; cmake .. ; $(MAKE) -j

$(BDIR)/.f:
	@mkdir -p $(dir $@)
	@touch $@

.PRECIOUS: %/.f

reformat:
	@perl -i -pe 's/	/    /g' $$(find -name '*.h' -o -name '*.cpp')
	@for f in $$(find -name '*.h' -o -name '*.cpp') ; do echo $$f ; clang-format -style=file -i $$f ; done
	@perl -i -pe 's/\s\s*$$/\n/g' $$(find -name '*.h' -o -name '*.cpp')

clean:
	@ rm -f *~ *.o *.bc *.ii *.s $(TARGETS)
	@ rm -f */*~ */*.o */*.bc */*.ii */*.s $(TARGETS)
	@ rm -rf build cbuild
