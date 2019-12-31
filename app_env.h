
#pragma once

#include <singleton.h>

#include <iostream>
#include <unistd.h>    // extern "C" char **environ;

namespace es::init {

class app_env {
	// unordered_map<std::string, std::string> _env{};
	static inline unsigned env_count{0};

public:
	app_env() {
		unsigned n{0};
		while(::environ[n]) {
			; // std::cout << "app env[" << n << "]: " << environ[n] << std::endl;
			++n;
		}
		env_count = n;
	}

	template<typename F>
	void for_each(F&& f) {
		for (auto ii = 0; ::environ[ii]; ++ii)
			f(ii, ::environ[ii]);
	}
};

[[ using gnu : used ]] static inline auto & env { singleton<app_env, early_args_initializer>::instance() };

}
