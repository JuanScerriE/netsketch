#pragma once

// cstd
#include <cstdint>

// std
#include <string>
#include <variant>
#include <vector>

struct Colour {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct LineDraw {
    Colour colour;
    int x0;
    int y0;
    int x1;
    int y1;
};

struct RectangleDraw {
    Colour colour;
    int x;
    int y;
    int w;
    int h;
};

struct CircleDraw {
    Colour colour;
    int x;
    int y;
    float r;
};

struct TextDraw {
    Colour colour;
    int x;
    int y;
    std::string string {};
};

enum class DrawType : std::uint16_t {
    LINE,
    RECTANGLE,
    CIRCLE,
    TEXT
};

using Draw = std::variant<LineDraw, RectangleDraw, CircleDraw, TextDraw>;

struct Select {
    long id {};
    Draw draw {};
};

struct Delete {
    long id {};
};

struct Undo { };

enum class Qualifier : std::uint8_t {
    ALL,
    MINE
};

struct Clear {
    Qualifier qualifier {};
};

using Action = std::variant<Draw, Select, Delete, Undo, Clear>;

enum class ActionType : std::uint16_t {
    DRAW,
    SELECT,
    DELETE,
    UNDO,
    CLEAR
};

struct TaggedAction {
    std::string username {};
    Action action {};
};

struct TaggedDraw {
    bool adopted {};
    std::string username {};
    Draw draw {};
};

using TaggedDrawVector = std::vector<TaggedDraw>;

struct Username {
    std::string username {};
};

struct Accept { };

struct Decline {
    std::string reason {};
};

struct Adopt {
    std::string username {};
};

enum class PayloadType : std::uint16_t {
    TAGGED_ACTION,
    TAGGED_DRAW_VECTOR,
    USERNAME,
    ACCEPT,
    DECLINE,
    ADOPT
};

using Payload = std::
    variant<TaggedAction, TaggedDrawVector, Username, Accept, Decline, Adopt>;
