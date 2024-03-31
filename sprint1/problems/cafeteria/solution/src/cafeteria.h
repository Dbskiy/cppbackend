#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

#include "hotdog.h"
#include "result.h"
#include <syncstream>
#include <chrono>
#include <syncstream>

using namespace std::literals;
using namespace std::chrono;

namespace net = boost::asio;

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;
using Timer = net::steady_timer;

class Order : public std::enable_shared_from_this<Order> {
public:
	Order(net::io_context& io, int id, std::shared_ptr<GasCooker> gas_cooker, std::shared_ptr<Sausage> sausage, std::shared_ptr<Bread> bread, HotDogHandler handler)
		: io_{ io }
		, id_{ id }
		, gas_cooker_{ gas_cooker }
		, sausage_{ sausage }
		, bread_{ bread }
		, handler_{ handler }
	{
	}
	// Запускает асинхронное выполнение заказа
	void Execute() {
		sausage_->StartFry(*gas_cooker_, [self = shared_from_this()] {
			net::dispatch(self->io_, [self] {
				self->frying_timer_.expires_after(1500ms);
				self->frying_timer_.async_wait([self, sausage = self->sausage_->shared_from_this(), bread = self->bread_->shared_from_this()](sys::error_code ec) {
					if (ec) throw std::runtime_error(ec.what());
					sausage->StopFry();

					if (bread->IsCooked()) {
						self->handler_(HotDog(self->id_, std::move(sausage), std::move(bread)));
					}
					});
				});
			});

		bread_->StartBake(*gas_cooker_, [self = shared_from_this()] {
			net::dispatch(self->io_, [self] {
				self->baking_timer_.expires_after(1000ms);
				self->baking_timer_.async_wait([self, sausage = self->sausage_->shared_from_this(), bread = self->bread_->shared_from_this()](sys::error_code ec)
					{
						if (ec) throw std::runtime_error(ec.what());
						bread->StopBake();
						if (sausage->IsCooked()) {
							self->handler_(HotDog(self->id_, std::move(sausage), std::move(bread)));
						}
					});
				});
			});
	}

private:

	net::io_context& io_;
	net::strand<net::io_context::executor_type> strand_{ net::make_strand(io_) };
	int id_;
	std::shared_ptr<GasCooker> gas_cooker_;
	std::shared_ptr<Sausage> sausage_;
	std::shared_ptr<Bread> bread_;
	Timer frying_timer_{ strand_ };
	Timer baking_timer_{ strand_ };
	HotDogHandler handler_;
};


// Класс "Кафетерий". Готовит хот-доги
class Cafeteria {
public:
	using osync = std::osyncstream;
	explicit Cafeteria(net::io_context& io)
		: io_{ io } {
	}

	// Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
   // Этот метод может быть вызван из произвольного потока
	void OrderHotDog(HotDogHandler handler) {
		net::post(strand, [=] {
		std::shared_ptr<Sausage> sausage_ = store_.GetSausage();
		std::shared_ptr<Bread> bread_ = store_.GetBread();
		std::shared_ptr<GasCooker> gas_cooker = gas_cooker_;
		const int order_id = ++next_id_;
			std::make_shared<Order>(io_, order_id, gas_cooker, sausage_, bread_, handler)->Execute();
			});
	}

private:
	net::io_context& io_;
	net::strand<net::io_context::executor_type> strand{ net::make_strand(io_) };
	// Используется для создания ингредиентов хот-дога
	Store store_;
	// Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
	// Используйте её для приготовления ингредиентов хот-дога.
	// Плита создаётся с помощью make_shared, так как GasCooker унаследован от
	// enable_shared_from_this.
	std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
	static int next_id_;
};
int Cafeteria::next_id_ = 0;

