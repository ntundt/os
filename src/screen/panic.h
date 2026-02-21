#ifndef PANIC_H
#define PANIC_H

__attribute__((noreturn)) void panic(const char *message);

#define stringify(x) stringify2(x)
#define stringify2(x) #x
#define assert(x) if (!(x)) panic("Assertion \"" #x "\" failed in " __FILE__ \
	", line " stringify(__LINE__) )

#endif
