CC=g++
DFLAGS=-D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr
RFLAGS=-D NDEBUG -Wall
SOURCES=main.cpp tree/tree.cpp tree_transforms.cpp latexing.cpp lexer.cpp recursive_reader.cpp
DOBJECTS=$(addprefix debug_object_files/,$(notdir $(SOURCES:.cpp=.o)))
ROBJECTS=$(addprefix release_object_files/,$(notdir $(SOURCES:.cpp=.o)))
DEXECUTABLE=debug
REXECUTABLE=diff

all: $(SOURCES) $(DEXECUTABLE)

$(DEXECUTABLE): $(DOBJECTS)
	$(CC) $(DFLAGS) $(DOBJECTS) -o $@

debug_object_files/%.o: %.cpp
	$(CC) -c $(DFLAGS) $< -o $@

debug_object_files/%.o: tree/%.cpp
	$(CC) -c $(DFLAGS) $< -o $@

clean:
	rm -rf *.o
	rm -rf debug_object_files/*.o
	rm -rf release_object_files/*.o

texclean:
	rm -rf *.tex
	rm -rf *.aux
	rm -rf *.log
	rm -rf latex/*.tex
	rm -rf latex/*.aux
	rm -rf latex/*.log

release: $(SOURCES) $(REXECUTABLE)

$(REXECUTABLE): $(ROBJECTS)
	$(CC) $(RFLAGS) $(ROBJECTS) -o $@

release_object_files/%.o: %.cpp
	$(CC) -c $(RFLAGS) $< -o $@

release_object_files/%.o: tree/%.cpp
	$(CC) -c $(RFLAGS) $< -o $@
