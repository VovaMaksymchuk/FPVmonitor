#pragma once
#include <Arduino.h>

enum class Key : uint8_t { None, Up, Down, Ok, OkLong };

struct KeyEvent {
    Key key = Key::None;
    uint16_t repeat = 0;  // 0 = first press, >0 = auto-repeat while held
};

// UP/DOWN: event on press, then auto-repeat (accelerating) while held.
// OK: Ok on release of a short press, OkLong once when held long enough.
class Buttons {
public:
    void begin();
    void update();
    KeyEvent poll();

private:
    struct State {
        int pin = -1;
        bool down = false;
        bool raw = false;
        uint32_t changedAt = 0;
        uint32_t pressedAt = 0;
        uint32_t nextRepeat = 0;
        uint16_t repeat = 0;
        bool longFired = false;
    };

    void push(Key key, uint16_t repeat);
    void updateArrow(State &s, Key key, uint32_t now);
    void updateOk(State &s, uint32_t now);
    // Returns true when the debounced state changed.
    bool debounce(State &s, uint32_t now);

    State up_, ok_, down_;

    static constexpr int QUEUE = 8;
    KeyEvent queue_[QUEUE];
    uint8_t head_ = 0, tail_ = 0;
};
