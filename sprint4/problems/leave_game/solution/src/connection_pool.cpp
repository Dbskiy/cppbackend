#include "connection_pool.h"

namespace conn_pool {
    ConnectionPool::ConnectionWrapper ConnectionPool::GetConnection()
	{
        std::unique_lock lock{ mutex_ };
        // Ѕлокируем текущий поток и ждЄм, пока cond_var_ не получит уведомление и не освободитс€
        // хот€ бы одно соединение
        cond_var_.wait(lock, [this] {
            return used_connections_ < pool_.size();
            });
        // ѕосле выхода из цикла ожидани€ мьютекс остаЄтс€ захваченным

        return { std::move(pool_[used_connections_++]), *this };
	}

    void ConnectionPool::ReturnConnection(ConnectionPtr&& conn)
    {
        // ¬озвращаем соединение обратно в пул
        {
            std::lock_guard lock{ mutex_ };
            assert(used_connections_ != 0);
            pool_[--used_connections_] = std::move(conn);
        }
        // ”ведомл€ем один из ожидающих потоков об изменении состо€ни€ пула
        cond_var_.notify_one();
    }
} //namespace conn_pool