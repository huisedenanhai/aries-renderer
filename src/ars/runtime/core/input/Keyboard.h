#pragma once

namespace ars::input {
enum class Key {
    Space,
    Apostrophe,   /* ' */
    Comma,        /* , */
    Minus,        /* - */
    Period,       /* . */
    Slash,        /* / */
    Semicolon,    /* ; */
    Equal,        /* = */
    LeftBracket,  /* [ */
    BackSlash,    /* \ */
    RightBracket, /* ] */
    GraveAccent,  /* ` */

    // 0 - 9
    N0,
    N1,
    N2,
    N3,
    N4,
    N5,
    N6,
    N7,
    N8,
    N9,

    // A - Z
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    Escape,
    Enter,
    Tab,
    Backspace,
    Insert,
    Delete,
    PageUp,
    PageDown,
    Home,
    End,
    CapsLock,
    ScrollLock,
    NumLock,
    PrintScreen,
    Pause,

    // Arrows
    Right,
    Left,
    Down,
    Up,

    // F1 - F25
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    F25,

    // Key Pad
    KeyPad0,
    KeyPad1,
    KeyPad2,
    KeyPad3,
    KeyPad4,
    KeyPad5,
    KeyPad6,
    KeyPad7,
    KeyPad8,
    KeyPad9,
    KeyPadDecimal,
    KeyPadDivide,
    KeyPadMultiply,
    KeyPadSubtract,
    KeyPadAdd,
    KeyPadEnter,
    KeyPadEqual,

    LeftShift,
    LeftControl,
    LeftAlt,
    LeftSuper,
    RightShift,
    RightControl,
    RightAlt,
    RightSuper,

    Menu
};

class IKeyBoard {
  public:
    virtual ~IKeyBoard() = default;

    virtual bool is_holding(Key key) = 0;
    virtual bool is_pressed(Key key) = 0;
    virtual bool is_released(Key key) = 0;
};
} // namespace ars::input