#ifndef _CPUSUPPORT_H_
#define _CPUSUPPORT_H_

#ifdef CPUSUPPORT_CONFIG_FILE
#include CPUSUPPORT_CONFIG_FILE
#endif

#define CPUSUPPORT_FEATURE(arch, feature)				\
	extern int cpusupport_ ## arch ## _ ## feature ## _present;	\
	extern int cpusupport_ ## arch ## _ ## feature ## _init;	\
	int cpusupport_ ## arch ## _ ## feature ## _detect(void);	\
									\
	static inline int						\
	cpusupport_ ## arch ## _ ## feature(void)			\
	{								\
									\
		if (cpusupport_ ## arch ## _ ## feature ## _present)	\
			return (1);					\
		else if (cpusupport_ ## arch ## _ ## feature ## _init)	\
			return (0);					\
		cpusupport_ ## arch ## _ ## feature ## _present = 	\
		    cpusupport_ ## arch ##_ ## feature ## _detect();	\
		cpusupport_ ## arch ## _ ## feature ## _init = 1;	\
		return (cpusupport_ ## arch ## _ ## feature ## _present); \
	}								\
	struct cpusupport_ ## arch ## _ ## feature ## _dummy

CPUSUPPORT_FEATURE(x86, aesni);

#endif /* !_CPUSUPPORT_H_ */
