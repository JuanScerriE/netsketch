#pragma once

// cstd
#include <cstdint>

// std
#include <string>
#include <variant>
#include <vector>

struct Colour {
    uint8_t r { 0 };
    uint8_t g { 0 };
    uint8_t b { 0 };

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(r, g, b);
    }
};

struct LineDraw {
    Colour colour {};
    int x0 { 0 };
    int y0 { 0 };
    int x1 { 0 };
    int y1 { 0 };

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(colour, x0, y0, x1, y1);
    }
};

struct RectangleDraw {
    Colour colour {};
    int x { 0 };
    int y { 0 };
    int w { 0 };
    int h { 0 };

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(colour, x, y, w, h);
    }
};

struct CircleDraw {
    Colour colour {};
    int x { 0 };
    int y { 0 };
    float r { 0 };

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(colour, x, y, r);
    }
};

struct TextDraw {
    Colour colour {};
    int x { 0 };
    int y { 0 };
    std::string string {};

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(colour, x, y, string);
    }
};

using Draw = std::variant<LineDraw, RectangleDraw, CircleDraw, TextDraw>;

struct Select {
    long id {};
    Draw draw {};

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(id, draw);
    }
};

struct Delete {
    long id {};

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(id);
    }
};

struct Undo {
    template <class Archive>
    void serialize(Archive&)
    {
    }
};

enum class Qualifier : std::uint16_t {
    ALL,
    MINE
};

struct Clear {
    Qualifier qualifier {};

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(qualifier);
    }
};

using Action = std::variant<Draw, Select, Delete, Undo, Clear>;

struct TaggedAction {
    std::string username {};
    Action action {};

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(username, action);
    }
};

struct TaggedDraw {
    bool adopted {};
    std::string username {};
    Draw draw {};

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(adopted, username, draw);
    }
};

using TaggedDrawVector = std::vector<TaggedDraw>;

struct Username {
    std::string username {};

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(username);
    }
};

struct Accept {
    template <class Archive>
    void serialize(Archive&)
    {
    }
};

struct Decline {
    std::string reason {};

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(reason);
    }
};

struct Adopt {
    std::string username {};

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(username);
    }
};

using Payload = std::
    variant<TaggedAction, TaggedDrawVector, Username, Accept, Decline, Adopt>;
