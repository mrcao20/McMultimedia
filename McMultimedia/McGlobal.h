#ifndef _MC_GLOBAL_H_
#define _MC_GLOBAL_H_

// 用来删除指针，删除前会判断指针是否存在
#define MC_SAFE_DELETE(p) (p) ? ((delete p), (p = nullptr)) : (true)

// 用来调用指定函数，会判断指针是否存在
#define MC_SAFE_CALL(p, func) (p) ? (p->func) : (false)

// 循环调用，变量容器，调用指定的函数，容器内的类型只能是普通对象
#define MC_LOOP_CALL(container, func)	\
	for (const auto &c : container) {	\
		static_assert(!std::is_pointer<std::decay<decltype(c)>::type>::value, "Data type in container must not be a pointer type");	\
		c.func;	\
	}

// 同上，容器内的类型只能是指针
#define MC_LOOP_CALL_P(container, func)	\
	for (const auto &c : container) {	\
		static_assert(std::is_pointer<std::decay<decltype(c)>::type>::value, "Data type in container must be a pointer type");	\
		c->func;	\
	}

#endif // !_MC_GLOBAL_H_

