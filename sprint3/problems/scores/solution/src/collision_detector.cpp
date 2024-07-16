#include "collision_detector.h"
#include <cassert>

namespace collision_detector {

CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c) {
    // Проверим, что перемещение ненулевое.
    // Тут приходится использовать строгое равенство, а не приближённое,
    // пскольку при сборе заказов придётся учитывать перемещение даже на небольшое
    // расстояние.
    assert(b.x != a.x || b.y != a.y);
    const double u_x = c.x - a.x;
    const double u_y = c.y - a.y;
    const double v_x = b.x - a.x;
    const double v_y = b.y - a.y;
    const double u_dot_v = u_x * v_x + u_y * v_y;
    const double u_len2 = u_x * u_x + u_y * u_y;
    const double v_len2 = v_x * v_x + v_y * v_y;
    const double proj_ratio = u_dot_v / v_len2;
    const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

    return CollectionResult(sq_distance, proj_ratio);
}

std::vector<GatheringEvent> FindGatherEvents(
    const ItemGathererProvider& provider) {

    std::vector<GatheringEvent> events;

    for (size_t g = 0; g < provider.GatherersCount(); g++)
    {
        Gatherer gatherer = provider.GetGatherer(g);

        if ((gatherer.start_pos.x == gatherer.end_pos.x) && (gatherer.start_pos.y == gatherer.end_pos.y)) {
            continue;
        }

        for (size_t i = 0; i < provider.ItemsCount(); i++)
        {
            Item item = provider.GetItem(i);
            auto result = TryCollectPoint(gatherer.start_pos, gatherer.end_pos, item.position);

            if (result.IsCollected((gatherer.width + item.width) / 2)) {
                GatheringEvent event{ .item_id = i,
                                      .gatherer_id = g,
                                      .sq_distance = result.sq_distance,
                                      .time = result.proj_ratio };
                events.push_back(event);
            }
        }

    }

    std::sort(events.begin(), events.end(), [](const GatheringEvent& l, const GatheringEvent& r) {
        return l.time < r.time;
        });

    return events;
}
std::vector<DeliveringEvent> FindDeliveringEvents(const ItemsDelivererProvider& provider)
{
    std::vector<DeliveringEvent> events;
    for (size_t d = 0; d < provider.DeliverersCount(); d++) {
        Gatherer deliverer = provider.GetDeliverer(d);

        if ((deliverer.start_pos.x == deliverer.end_pos.x) && (deliverer.start_pos.y == deliverer.end_pos.y)) {
            continue;
        }

        for (size_t b = 0; b < provider.BasesCount(); b++)
        {
            Base base = provider.GetBase(d);
            auto result = TryCollectPoint(deliverer.start_pos, deliverer.end_pos, base.position);

            if (result.IsCollected((deliverer.width + base.width) / 2)) {
                DeliveringEvent event{.deliverer_id = d,
                                      .sq_distance = result.sq_distance,
                                      .time = result.proj_ratio };
                events.push_back(event);
            }
        }
    }

    std::sort(events.begin(), events.end(), [](const DeliveringEvent& l, const DeliveringEvent& r) {
        return l.time < r.time;
        });

    return events;

}
}  // namespace collision_detector