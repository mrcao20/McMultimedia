#ifndef _MC_GLOBAL_H_
#define _MC_GLOBAL_H_

#define MC_SAFE_DELETE(p) (p) ? ((delete p), (p = nullptr)) : (true)

#define MC_LOOP_CALL(container, func)	\
	for (const auto &c : container) {	\
		static_assert(!std::is_pointer<std::decay<decltype(c)>::type>::value, "Data type in container must not be a pointer type");	\
		c->func();	\
	}

#define MC_LOOP_CALL_P(container, func)	\
	for (const auto &c : container) {	\
		static_assert(std::is_pointer<std::decay<decltype(c)>::type>::value, "Data type in container must be a pointer type");	\
		c->func();	\
	}

#endif // !_MC_GLOBAL_H_

