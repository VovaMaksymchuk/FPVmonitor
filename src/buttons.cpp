#include "buttons.h"
#include "config.h"

void Buttons::begin() {
    up_.pin = PIN_BTN_UP;
    ok_.pin = PIN_BTN_OK;
    down_.pin = PIN_BTN_DOWN;
    for (State *s : {&up_, &ok_, &down_}) pinMode(s->pin, INPUT_PULLUP);
}

void Buttons::push(Key key, uint16_t repeat) {
    uint8_t next = (head_ + 1) % QUEUE;
    if (next == tail_) return;  // full, drop
    queue_[head_].key = key;
    queue_[head_].repeat = repeat;
    head_ = next;
}

KeyEvent Buttons::poll() {
    if (tail_ == head_) return KeyEvent();
    KeyEvent ev = queue_[tail_];
    tail_ = (tail_ + 1) % QUEUE;
    return ev;
}

bool Buttons::debounce(State &s, uint32_t now) {
    bool raw = digitalRead(s.pin) == LOW;
    if (raw != s.raw) {
        s.raw = raw;
        s.changedAt = now;
    }
    if (s.raw != s.down && now - s.changedAt >= BTN_DEBOUNCE_MS) {
        s.down = s.raw;
        return true;
    }
    return false;
}

void Buttons::updateArrow(State &s, Key key, uint32_t now) {
    if (debounce(s, now) && s.down) {
        s.pressedAt = now;
        s.repeat = 0;
        s.nextRepeat = now + BTN_REPEAT_DELAY_MS;
        push(key, 0);
        return;
    }
    if (s.down && (int32_t)(now - s.nextRepeat) >= 0) {
        s.repeat++;
        s.nextRepeat = now + (s.repeat > 10 ? BTN_REPEAT_FAST_MS : BTN_REPEAT_MS);
        push(key, s.repeat);
    }
}

void Buttons::updateOk(State &s, uint32_t now) {
    if (debounce(s, now)) {
        if (s.down) {
            s.pressedAt = now;
            s.longFired = false;
        } else if (!s.longFired) {
            push(Key::Ok, 0);
        }
        return;
    }
    if (s.down && !s.longFired && now - s.pressedAt >= BTN_LONG_MS) {
        s.longFired = true;
        push(Key::OkLong, 0);
    }
}

void Buttons::update() {
    uint32_t now = millis();
    updateArrow(up_, Key::Up, now);
    updateArrow(down_, Key::Down, now);
    updateOk(ok_, now);
}
