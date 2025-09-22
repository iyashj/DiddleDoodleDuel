#ifndef DIDDLEDOODLEDUEL_EVENT_DEFINITIONS_H
#define DIDDLEDOODLEDUEL_EVENT_DEFINITIONS_H

#include <string>

struct MenuEvent {
    enum class Type : uint8_t { StartLocalGame, StartOnlineGame, ExitGame, BackToMenu } type;
};




#endif // DIDDLEDOODLEDUEL_EVENT_DEFINITIONS_H
