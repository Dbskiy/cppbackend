#pragma once
#include <memory>
#include "../app/unit_of_work.h"


namespace app {
	class UnitOfWorkFactory {
	public:
		virtual std::unique_ptr<UnitOfWork> CreateUnitOfWork() = 0;

		virtual ~UnitOfWorkFactory() = default;
	};
}