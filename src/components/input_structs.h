#ifndef DIDDLEDOODLEDUEL_INPUTS_H
#define DIDDLEDOODLEDUEL_INPUTS_H

struct InputMapping {

    InputMapping(const KeyboardKey rotateLeftKey, const KeyboardKey rotateRightKey) :
    rotateLeftKey(rotateLeftKey), rotateRightKey(rotateRightKey){}

    KeyboardKey rotateLeftKey;
    KeyboardKey rotateRightKey;
};

struct InputAction {
    bool rotateLeft { false };
    bool rotateRight { false };
};

#endif // DIDDLEDOODLEDUEL_INPUTS_H
