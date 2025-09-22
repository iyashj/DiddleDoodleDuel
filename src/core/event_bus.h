#ifndef DIDDLEDOODLEDUEL_EVENT_BUS_H
#define DIDDLEDOODLEDUEL_EVENT_BUS_H

#include <entt/signal/dispatcher.hpp>

struct EventBus {
    entt::dispatcher dispatcher;
};

#endif // DIDDLEDOODLEDUEL_EVENT_BUS_H
