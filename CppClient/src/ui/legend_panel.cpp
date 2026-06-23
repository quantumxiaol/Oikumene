#include "oikumene/ui/legend_panel.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

#include "oikumene/render/symbol_registry.hpp"

namespace oikumene {
namespace {

void DrawLegendBackground(Rectangle bounds) {
    DrawRectangleRec(bounds, Color{18, 22, 26, 232});
    DrawRectangleLinesEx(bounds, 1.0F, Color{88, 98, 108, 230});
}

void DrawSymbolShape(const SymbolInfo& symbol, Vector2 center, float size) {
    const float half = size * 0.5F;
    const float radius = size * 0.42F;
    const Color outline{18, 22, 26, 235};

    if (symbol.shape == "circle") {
        DrawCircleV(center, radius, symbol.color);
        DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radius, outline);
        return;
    }
    if (symbol.shape == "triangle") {
        DrawTriangle(Vector2{center.x, center.y - half}, Vector2{center.x - half, center.y + half},
                     Vector2{center.x + half, center.y + half}, symbol.color);
        DrawTriangleLines(Vector2{center.x, center.y - half}, Vector2{center.x - half, center.y + half},
                          Vector2{center.x + half, center.y + half}, outline);
        return;
    }
    if (symbol.shape == "ring" || symbol.shape == "double_ring") {
        DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radius, symbol.color);
        if (symbol.shape == "double_ring") {
            DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radius * 1.35F, symbol.color);
        }
        return;
    }
    if (symbol.shape == "line") {
        DrawLineEx(Vector2{center.x - half, center.y}, Vector2{center.x + half, center.y}, 4.0F, symbol.color);
        return;
    }
    if (symbol.shape == "cross") {
        DrawLineEx(Vector2{center.x - half, center.y - half}, Vector2{center.x + half, center.y + half}, 3.0F,
                   symbol.color);
        DrawLineEx(Vector2{center.x + half, center.y - half}, Vector2{center.x - half, center.y + half}, 3.0F,
                   symbol.color);
        return;
    }
    if (symbol.shape == "outline") {
        DrawRectangleLinesEx(Rectangle{center.x - half, center.y - half, size, size}, 2.0F, symbol.color);
        return;
    }
    if (symbol.shape == "grid") {
        DrawRectangleRec(Rectangle{center.x - half, center.y - half, size, size}, symbol.color);
        DrawLineEx(Vector2{center.x - half * 0.72F, center.y}, Vector2{center.x + half * 0.72F, center.y}, 1.0F,
                   outline);
        DrawLineEx(Vector2{center.x, center.y - half * 0.72F}, Vector2{center.x, center.y + half * 0.72F}, 1.0F,
                   outline);
        return;
    }
    if (symbol.shape == "house") {
        DrawRectangleRec(Rectangle{center.x - half, center.y - half * 0.35F, size, half * 1.1F}, symbol.color);
        DrawTriangle(Vector2{center.x, center.y - half}, Vector2{center.x - half, center.y - half * 0.35F},
                     Vector2{center.x + half, center.y - half * 0.35F}, Color{170, 74, 48, 255});
        return;
    }
    if (symbol.shape == "star") {
        std::array<Vector2, 5> points{};
        for (int i = 0; i < 5; ++i) {
            const float angle = -1.5708F + static_cast<float>(i) * 6.28318F / 5.0F;
            points[static_cast<std::size_t>(i)] =
                Vector2{center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius};
        }
        for (int i = 0; i < 5; ++i) {
            DrawLineEx(points[static_cast<std::size_t>(i)], points[static_cast<std::size_t>((i + 2) % 5)], 2.0F,
                       symbol.color);
        }
        return;
    }

    DrawRectangleRec(Rectangle{center.x - half, center.y - half, size, size}, symbol.color);
    DrawRectangleLinesEx(Rectangle{center.x - half, center.y - half, size, size}, 1.0F, outline);
}

std::string TruncateText(std::string text, std::size_t limit) {
    if (text.size() <= limit) {
        return text;
    }
    return text.substr(0, limit > 3 ? limit - 3 : limit) + "...";
}

int DrawCategory(SymbolCategory category, int x, int y, int max_width) {
    DrawText(ToString(category).c_str(), x, y, 17, Color{240, 244, 248, 255});
    y += 26;

    for (const auto& symbol : LegendSymbolsFor(category)) {
        DrawSymbolShape(symbol, Vector2{static_cast<float>(x + 12), static_cast<float>(y + 9)}, 15.0F);
        DrawText(symbol.label.c_str(), x + 30, y, 15, Color{220, 225, 230, 255});
        const int desc_x = x + 148;
        DrawText(TruncateText(symbol.description, static_cast<std::size_t>(std::max(28, max_width / 8))).c_str(), desc_x,
                 y + 1, 13, Color{156, 168, 178, 255});
        y += 22;
    }
    return y + 10;
}

}  // namespace

void DrawLegendPanel(Rectangle bounds) {
    DrawLegendBackground(bounds);
    const int x = static_cast<int>(bounds.x) + 18;
    int y = static_cast<int>(bounds.y) + 16;
    const int column_width = static_cast<int>((bounds.width - 54.0F) * 0.5F);
    const int right_x = x + column_width + 18;

    DrawText("Legend / 图例", x, y, 22, RAYWHITE);
    DrawText("F2 close", static_cast<int>(bounds.x + bounds.width - 88.0F), y + 4, 14, Color{156, 168, 178, 255});
    y += 34;

    int left_y = y;
    int right_y = y;
    left_y = DrawCategory(SymbolCategory::Biome, x, left_y, column_width);
    left_y = DrawCategory(SymbolCategory::Water, x, left_y, column_width);
    right_y = DrawCategory(SymbolCategory::Resource, right_x, right_y, column_width);
    right_y = DrawCategory(SymbolCategory::Improvement, right_x, right_y, column_width);
    left_y = DrawCategory(SymbolCategory::Entity, x, left_y, column_width);
    right_y = DrawCategory(SymbolCategory::Polity, right_x, right_y, column_width);
    right_y = DrawCategory(SymbolCategory::Overlay, right_x, right_y, column_width);
    (void)left_y;
    (void)right_y;
}

}  // namespace oikumene
