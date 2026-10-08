#include "settings.h"
#include <algorithm>
namespace arcade {
Settings default_settings() {
    Settings settings;
    std::fill_n(reinterpret_cast<uint8_t *>(&settings), sizeof(settings), 0);
    settings = Settings{};
    std::copy(std::begin(DefaultKeys), std::end(DefaultKeys), settings.bindings);
    std::copy(std::begin(snake::DefaultFood), std::end(snake::DefaultFood), settings.food);
    return settings;
}
bool valid_settings(const Settings &settings) {
    if (settings.brightness < 5 || settings.brightness > 100 || settings.master > 100 ||
        settings.music > 100 || settings.effects > 100 || settings.music_on > 1 ||
        settings.effects_on > 1 || settings.mute > 1 || settings.theme > 2 || settings.fps > 1 ||
        settings.grid > 1 || !std::memchr(settings.player, 0, sizeof(settings.player)))
        return false;
    for (unsigned i = 0; i < size_t(Binding::Count); ++i) {
        if (!settings.bindings[i])
            return false;
        for (unsigned j = i + 1; j < size_t(Binding::Count); ++j)
            if (settings.bindings[i] == settings.bindings[j])
                return false;
    }
    for (const auto &food : settings.food)
        if (food.points > 10000 || food.growth > 100 || food.lifetime_ms > 60000)
            return false;
    if (settings.food[0].lifetime_ms != 0)
        return false;
    return true;
}
bool load_settings(Journal &journal, Settings &settings) {
    std::vector<uint8_t> bytes;
    Settings copy;
    if (!journal.read("config", 1, bytes) || !decode(bytes, copy) || !valid_settings(copy))
        return false;
    std::memcpy(&settings, &copy, sizeof(settings));
    return true;
}
bool save_settings(Journal &journal, const Settings &settings) {
    return valid_settings(settings) && journal.write("config", 1, snapshot(settings));
}
} // namespace arcade
