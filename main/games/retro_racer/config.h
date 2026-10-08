#pragma once
#include <cstdint>
namespace arcade::racer {
constexpr float RoadWidth = 360, PlayerY = 465, CarWidth = 36, CarHeight = 64;
constexpr unsigned MaxTraffic = 18;
struct Vehicle {
    const char *name;
    uint32_t color;
    float maximum, acceleration, steering, health;
};
constexpr Vehicle Vehicles[5] = {{"Vermelho / Equilibrado", 0xf75c65, 235, 85, 190, 100},
                                 {"Azul / Velocidade", 0x56a2ff, 280, 70, 170, 90},
                                 {"Verde / Controle", 0x4dde98, 215, 80, 260, 100},
                                 {"Amarelo / Aceleracao", 0xffd166, 240, 125, 185, 90},
                                 {"Roxo / Resistencia", 0xbf82ef, 215, 70, 175, 160}};
} // namespace arcade::racer
