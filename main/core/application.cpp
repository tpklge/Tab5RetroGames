#include "application.h"
#include "keyboard_manager.h"
#include "audio_manager.h"
#include "display_manager.h"
#include "lvgl_canvas.h"
#include "games/block_drop/render.h"
#include "games/snake/render.h"
#include "games/retro_racer/render.h"
#include <algorithm>
#include <cstdio>
#include <ctime>
namespace arcade {
namespace {
const char *difficulty(unsigned game, unsigned index) {
    static const char *names[3][4] = {{"Iniciante", "Normal", "Dificil", "Expert"},
                                      {"Facil", "Normal", "Dificil", "Insano"},
                                      {"Facil", "Normal", "Dificil", "Expert"}};
    return names[std::min(game, 2u)][index % 4];
}
std::string key_name(uint8_t code) {
    if (code == key::Left)
        return "Esquerda";
    if (code == key::Right)
        return "Direita";
    if (code == key::Up)
        return "Cima";
    if (code == key::Down)
        return "Baixo";
    if (code == key::Enter)
        return "Enter";
    if (code == key::Escape)
        return "Esc";
    if (code == key::Space)
        return "Espaco";
    if (code == key::Backspace)
        return "Backspace";
    char character = key_character(code, true);
    if (character)
        return std::string(1, character);
    char text[20];
    std::snprintf(text, sizeof(text), "Tecla 0x%02X", code);
    return text;
}
int cycle(int value, int direction, int count) { return (value + direction + count) % count; }
} // namespace
void Application::init() {
    load_settings(journal, settings);
    apply_settings();
    if (!keyboard::available())
        notice = "Conecte o teclado oficial e reinicie para jogar.";
    if (!audio::available())
        notice +=
            (notice.empty() ? "" : " ") + std::string("Som indisponivel nesta inicializacao.");
    show(Screen::Home);
}
void Application::apply_settings() {
    display::brightness(settings.brightness);
    audio::configure(settings.master, settings.music_on ? settings.music : 0,
                     settings.effects_on ? settings.effects : 0, settings.mute);
    audio::menu_music(settings.music_on && current != Screen::Playing);
}
void Application::persist_settings() {
    notice = save_settings(journal, settings)
                 ? "Configuracoes salvas."
                 : "Nao foi possivel salvar. Confira o espaco de armazenamento.";
    apply_settings();
}
std::string Application::save_key(unsigned game) const {
    return "save" + std::to_string(unsigned(registry()[game].id));
}
bool Application::save_exists(unsigned game) {
    std::vector<uint8_t> bytes;
    return journal.read(save_key(game), 3, bytes);
}
void Application::confirm(const char *message, std::function<void()> action, Screen back) {
    notice = message;
    confirmed = std::move(action);
    return_screen = back;
    show(Screen::Confirm);
}
void Application::show(Screen screen, unsigned selection) {
    if (surface)
        lv_obj_delete(surface);
    surface = fps_label = nullptr;
    current = screen;
    if (screen == Screen::Playing) {
        game_screen();
        return;
    }
    audio::engine(0);
    audio::menu_music(settings.music_on);
    std::vector<MenuItem> items;
    std::string description = notice;
    std::string title = "TAB5 RETRO ARCADE";
    const auto &selected = registry()[selected_game];
    auto go = [this](Screen target) {
        return [this, target] {
            notice.clear();
            show(target);
        };
    };
    auto value = [&](std::string label, std::function<void(int)> adjust) {
        items.push_back({std::move(label), [adjust] { adjust(1); }, std::move(adjust)});
    };
    if (screen == Screen::Home) {
        if (description.empty())
            description = "Pequenos jogos, grandes partidas. Escolha sua proxima aventura.";
        items = {{"Jogar", go(Screen::Games)},
                 {"Continuar partida",
                  [this] {
                      std::vector<MenuItem> saves;
                      for (unsigned index = 0; index < registry().size(); ++index)
                          if (save_exists(index))
                              saves.push_back({registry()[index].name, [this, index] {
                                                   selected_game = index;
                                                   start(true);
                                               }});
                      saves.push_back({"Voltar", [this] { show(Screen::Home); }});
                      notice.clear();
                      current = Screen::Games;
                      menu.show("CONTINUAR PARTIDA",
                                saves.size() > 1 ? "Selecione uma partida salva."
                                                 : "Nenhuma partida salva.",
                                std::move(saves), 0, settings.theme);
                  }},
                 {"Recordes", go(Screen::Records)},
                 {"Configuracoes", go(Screen::Settings)},
                 {"Sobre", go(Screen::About)}};
    } else if (screen == Screen::Games) {
        title = "ESCOLHA SEU JOGO";
        for (unsigned index = 0; index < registry().size(); ++index)
            items.push_back(
                {std::string(registry()[index].name) + "\n" + registry()[index].description,
                 [this, index] {
                     selected_game = index;
                     options = Options{};
                     notice.clear();
                     show(Screen::Setup);
                 },
                 {},
                 int(index % 3)});
        items.push_back({"Voltar", go(Screen::Home)});
    } else if (screen == Screen::Setup) {
        title = selected.name;
        if (description.empty())
            description = selected.description;
        items.push_back({"Iniciar nova partida", [this] {
                             if (save_exists(selected_game))
                                 confirm(
                                     "Substituir a partida anterior?", [this] { start(false); },
                                     Screen::Setup);
                             else
                                 start(false);
                         }});
        if (save_exists(selected_game))
            items.push_back({"Continuar partida salva", [this] { start(true); }});
        value(std::string("Modo: ") + selected.modes[options.mode], [this](int direction) {
            options.mode = cycle(options.mode, direction, 4);
            show(Screen::Setup, menu.selection());
        });
        value(std::string("Dificuldade: ") + difficulty(selected_game, options.difficulty),
              [this](int direction) {
                  options.difficulty = cycle(options.difficulty, direction, 4);
                  show(Screen::Setup, menu.selection());
              });
        if (selected.id == GameId::Snake) {
            const char *maps[] = {"Paredes", "Wrap around", "Obstaculos", "Labirinto"};
            value(std::string("Mapa: ") + maps[options.map], [this](int direction) {
                options.map = cycle(options.map, direction, 4);
                show(Screen::Setup, menu.selection());
            });
            items.push_back({"Configurar alimentos", go(Screen::Food)});
        }
        if (selected.id == GameId::Racer)
            value(std::string("Carro: ") + racer::Vehicles[options.vehicle].name,
                  [this](int direction) {
                      options.vehicle = cycle(options.vehicle, direction, 5);
                      show(Screen::Setup, menu.selection());
                  });
        items.push_back({"Recordes deste jogo", go(Screen::Records)});
        items.push_back({"Controles", go(Screen::Controls)});
        if (save_exists(selected_game))
            items.push_back({"Excluir partida salva", [this] {
                                 confirm(
                                     "Excluir a partida salva deste jogo?",
                                     [this] {
                                         notice = journal.erase(save_key(selected_game))
                                                      ? "Partida excluida."
                                                      : "Falha ao excluir a partida.";
                                         show(Screen::Setup);
                                     },
                                     Screen::Setup);
                             }});
        items.push_back({"Voltar", go(Screen::Games)});
    } else if (screen == Screen::Controls) {
        title = std::string("CONTROLES / ") + selected.name;
        description = selected.controls;
        description += "\nP / Esc: pausa. R: reiniciar com confirmacao. M: som.\nO mapeamento pode "
                       "ser alterado nas configuracoes.";
        items = {{"Voltar", go(Screen::Setup)}};
    } else if (screen == Screen::Records) {
        title = "TOP 10 / " + std::string(selected.name);
        value("Jogo: " + std::string(selected.name), [this](int direction) {
            selected_game = cycle(int(selected_game), direction, int(registry().size()));
            show(Screen::Records);
        });
        value("Modo: " + std::string(selected.modes[options.mode]), [this](int direction) {
            options.mode = cycle(options.mode, direction, 4);
            show(Screen::Records, 1);
        });
        value("Dificuldade: " + std::string(difficulty(selected_game, options.difficulty)),
              [this](int direction) {
                  options.difficulty = cycle(options.difficulty, direction, 4);
                  show(Screen::Records, 2);
              });
        Leaderboard board;
        records.load(selected.id, options.mode, options.difficulty, board);
        if (!board.count)
            items.push_back({"Nenhum recorde nesta categoria.", {}, {}});
        for (unsigned i = 0; i < board.count; ++i) {
            char row[200];
            auto &record = board.entries[i];
            char date[24]{};
            if (record.date) {
                std::time_t timestamp = record.date;
                std::tm calendar{};
                if (gmtime_r(&timestamp, &calendar))
                    std::strftime(date, sizeof(date), "  %Y-%m-%d", &calendar);
            }
            std::snprintf(row, sizeof(row), "%02u. %-15s  %lu pts  Nivel %u  %lu s  %lu m%s", i + 1,
                          record.player, (unsigned long)record.score, record.level,
                          (unsigned long)(record.time_ms / 1000), (unsigned long)record.distance,
                          record.won ? "  CONCLUIDO" : "");
            std::strncat(row, date, sizeof(row) - std::strlen(row) - 1);
            items.push_back({row, {}, {}});
        }
        items.push_back({"Apagar recordes desta categoria", [this] {
                             confirm(
                                 "Apagar os 10 recordes desta categoria?",
                                 [this] {
                                     notice = records.erase(registry()[selected_game].id,
                                                            options.mode, options.difficulty)
                                                  ? "Recordes apagados."
                                                  : "Falha ao apagar recordes.";
                                     show(Screen::Records);
                                 },
                                 Screen::Records);
                         }});
        items.push_back({"Voltar", go(Screen::Home)});
    } else if (screen == Screen::Settings) {
        title = "CONFIGURACOES";
        auto number = [&](const char *label, uint8_t *field, int min, int max, int step) {
            value(std::string(label) + ": " + std::to_string(*field),
                  [this, field, min, max, step](int direction) {
                      *field = uint8_t(std::clamp(int(*field) + direction * step, min, max));
                      unsigned selected = menu.selection();
                      persist_settings();
                      show(Screen::Settings, selected);
                  });
        };
        auto toggle = [&](const char *label, uint8_t *field) {
            value(std::string(label) + ": " + (*field ? "Sim" : "Nao"), [this, field](int) {
                *field = !*field;
                unsigned selected = menu.selection();
                persist_settings();
                show(Screen::Settings, selected);
            });
        };
        number("Brilho", &settings.brightness, 5, 100, 5);
        number("Volume geral", &settings.master, 0, 100, 5);
        number("Volume musica", &settings.music, 0, 100, 5);
        number("Volume efeitos", &settings.effects, 0, 100, 5);
        toggle("Musica ligada", &settings.music_on);
        toggle("Efeitos ligados", &settings.effects_on);
        toggle("Mudo", &settings.mute);
        const char *themes[] = {"Arcade", "Neon", "Grafite"};
        value(std::string("Tema: ") + themes[settings.theme], [this](int direction) {
            settings.theme = cycle(settings.theme, direction, 3);
            unsigned selected = menu.selection();
            persist_settings();
            show(Screen::Settings, selected);
        });
        toggle("Mostrar FPS medido", &settings.fps);
        toggle("Grade Block Drop", &settings.grid);
        items.push_back(
            {std::string("Nome do jogador: ") + settings.player, [this] { name_editor(false); }});
        items.push_back({"Mapeamento de controles", [this] {
                             remap = -1;
                             show(Screen::Remap);
                         }});
        if (backup)
            items.push_back({"Backup no microSD", [this] {
                                 notice = backup();
                                 show(Screen::Settings, menu.selection());
                             }});
        items.push_back({"Restaurar configuracoes padrao", [this] {
                             confirm(
                                 "Restaurar as configuracoes? Saves e recordes serao preservados.",
                                 [this] {
                                     settings = default_settings();
                                     persist_settings();
                                     show(Screen::Settings);
                                 },
                                 Screen::Settings);
                         }});
        items.push_back({"Voltar", go(Screen::Home)});
    } else if (screen == Screen::Remap) {
        title = "MAPEAMENTO DE CONTROLES";
        const char *names[] = {"Esquerda",  "Direita", "Cima / Girar horario", "Baixo",
                               "Confirmar", "Voltar",  "Acao principal",       "Pausar",
                               "Reiniciar", "Mudo",    "Girar anti-horario",   "Reservar peca"};
        if (remap >= 0)
            description = "Pressione a nova tecla. Esc cancela. Evite teclas ja utilizadas.";
        for (unsigned i = 0; i < size_t(Binding::Count); ++i)
            items.push_back(
                {std::string(names[i]) + ": " + key_name(settings.bindings[i]), [this, i] {
                     remap = int(i);
                     show(Screen::Remap, i);
                 }});
        items.push_back({"Voltar", go(Screen::Settings)});
    } else if (screen == Screen::Food) {
        title = "SNAKE / ALIMENTOS";
        description = "Ajuste pontos, crescimento e duracao. A comida normal nao expira.";
        const char *names[] = {"Normal",     "Especial", "Raro",
                               "Congelante", "Bonus x2", "Super bonus"};
        for (unsigned type = 0; type < 6; ++type) {
            value(std::string(names[type]) +
                      " / Pontos: " + std::to_string(settings.food[type].points),
                  [this, type](int direction) {
                      settings.food[type].points = uint16_t(
                          std::clamp(int(settings.food[type].points) + direction * 5, 0, 10000));
                      unsigned index = menu.selection();
                      persist_settings();
                      show(Screen::Food, index);
                  });
            value(std::string(names[type]) +
                      " / Crescimento: " + std::to_string(settings.food[type].growth),
                  [this, type](int direction) {
                      settings.food[type].growth =
                          uint16_t(std::clamp(int(settings.food[type].growth) + direction, 0, 100));
                      unsigned index = menu.selection();
                      persist_settings();
                      show(Screen::Food, index);
                  });
            if (type)
                value(std::string(names[type]) + " / Duracao: " +
                          std::to_string(settings.food[type].lifetime_ms / 1000) + " s",
                      [this, type](int direction) {
                          settings.food[type].lifetime_ms = uint32_t(
                              std::clamp(int(settings.food[type].lifetime_ms) + direction * 1000,
                                         1000, 60000));
                          unsigned index = menu.selection();
                          persist_settings();
                          show(Screen::Food, index);
                      });
        }
        items.push_back({"Voltar", go(Screen::Setup)});
    } else if (screen == Screen::About) {
        title = "SOBRE O ARCADE";
        description =
            "Tab5 Retro Arcade / Pipoca5\nTres jogos 2D offline para M5Stack Tab5.\nCodigo "
            "original MIT. Componentes M5Stack e Espressif com suas licencas.\nBlock Drop, Snake e "
            "Retro Racer.\nTeclado fisico e toque nos menus. Sons sintetizados originais.\nFPS "
            "exibido e medido pelo renderer; confirmar desempenho no Tab5.";
        items = {{"Voltar", go(Screen::Home)}};
    } else if (screen == Screen::Pause) {
        title = "PARTIDA PAUSADA";
        items = {{"Continuar",
                  [this] {
                      active->resume();
                      show(Screen::Playing);
                  }},
                 {"Salvar partida",
                  [this] {
                      save_active();
                      show(Screen::Pause);
                  }},
                 {"Reiniciar",
                  [this] {
                      confirm(
                          "Reiniciar esta partida?",
                          [this] {
                              active->reset();
                              record_pending = false;
                              show(Screen::Playing);
                          },
                          Screen::Pause);
                  }},
                 {"Voltar ao menu e salvar",
                  [this] {
                      if (save_active()) {
                          active->cleanup();
                          active = nullptr;
                          show(Screen::Home);
                      } else
                          show(Screen::Pause);
                  }},
                 {"Sair sem salvar", [this] {
                      confirm(
                          "Descartar a partida atual sem salvar?",
                          [this] {
                              active->cleanup();
                              active = nullptr;
                              show(Screen::Home);
                          },
                          Screen::Pause);
                  }}};
    } else if (screen == Screen::Confirm) {
        title = "CONFIRMAR";
        items = {{"Cancelar", [this] { show(return_screen); }}, {"Confirmar", [this] {
                                                                     auto action = confirmed;
                                                                     confirmed = {};
                                                                     if (action)
                                                                         action();
                                                                 }}};
    } else if (screen == Screen::Name) {
        title = naming_record ? "REGISTRAR PARTIDA" : "NOME DO JOGADOR";
        description = "Digite pelo teclado. Backspace apaga. Enter confirma. Esc volta.";
        items = {{std::string("Nome: ") + name, {}, {}},
                 {"Confirmar nome", [this] { submit_name(); }},
                 {"Voltar", [this] {
                      if (naming_record)
                          confirm(
                              "Voltar ao menu sem registrar esta partida?",
                              [this] {
                                  active->cleanup();
                                  active = nullptr;
                                  record_pending = false;
                                  show(Screen::Home);
                              },
                              Screen::Name);
                      else
                          show(Screen::Settings);
                  }}};
    }
    if (!notice.empty() && description.find(notice) == std::string::npos)
        description = notice + "\n" + description;
    menu.show(title.c_str(), description, std::move(items), selection, settings.theme);
}
void Application::start(bool restore) {
    if (active)
        active->cleanup();
    active = registry()[selected_game].create();
    if (restore) {
        std::vector<uint8_t> bytes;
        if (!journal.read(save_key(selected_game), 3, bytes) || !active->load(bytes)) {
            notice = "Partida invalida ou corrompida. A configuracao atual foi preservada.";
            active->cleanup();
            active = nullptr;
            show(Screen::Setup);
            return;
        }
        options = active->options();
        if (active->status() == Status::Lost || active->status() == Status::Won) {
            notice = "Esta partida ja terminou. Inicie uma nova partida.";
            show(Screen::Setup);
            return;
        }
        active->resume();
    } else {
        options.seed = uint32_t(lv_tick_get() * 1664525u + 1013904223u);
        if (!options.seed)
            options.seed = 1;
        active->init(options);
        active->start();
        if (registry()[selected_game].id == GameId::Snake) {
            auto *snake_game = static_cast<snake::SnakeGame *>(active);
            std::copy(std::begin(settings.food), std::end(settings.food),
                      snake_game->model.state.rules);
        }
        if (registry()[selected_game].id == GameId::BlockDrop)
            static_cast<block::BlockGame *>(active)->model.state.grid = settings.grid;
    }
    record_pending = false;
    finished_since = 0;
    notice.clear();
    input.clear();
    pending = {};
    show(Screen::Playing);
}
void Application::game_screen() {
    menu.close();
    audio::menu_music(false);
    surface = lv_obj_create(lv_screen_active());
    lv_obj_set_size(surface, 1280, 720);
    lv_obj_set_pos(surface, 0, 0);
    lv_obj_set_style_bg_opa(surface, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(surface, 0, 0);
    lv_obj_set_style_pad_all(surface, 0, 0);
    lv_obj_remove_flag(surface, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(surface, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(
        surface,
        [](lv_event_t *event) {
            auto *application = static_cast<Application *>(lv_event_get_user_data(event));
            application->draw(lv_event_get_layer(event));
        },
        LV_EVENT_DRAW_MAIN, this);
    fps_label = lv_label_create(surface);
    lv_obj_align(fps_label, LV_ALIGN_TOP_RIGHT, -20, 20);
    lv_obj_set_style_text_color(fps_label, lv_color_hex(color::Muted), 0);
    if (!settings.fps)
        lv_obj_add_flag(fps_label, LV_OBJ_FLAG_HIDDEN);
    accumulator = 0;
    step_fraction = 0;
    last_frame = 0;
    fps_since = previous_us;
    frame_count = display::refreshes();
    visual_valid = false;
}
void Application::draw(lv_layer_t *layer) {
    LvglCanvas canvas(layer);
    const uint32_t backgrounds[] = {color::Bg, 0x210e35, 0x10181b};
    canvas.rect(0, 0, 1280, 720, backgrounds[settings.theme]);
    canvas.text(35, 20, "TAB5 RETRO ARCADE", color::Cyan, Font::Pixel);
    canvas.text(380, 20, registry()[selected_game].modes[options.mode], color::Muted, Font::Small);
    if (active)
        active->render(canvas);
    canvas.text(35, 690, "P / Esc: pausa     R: reiniciar     M: som", color::Muted, Font::Small);
    if (active && (active->status() == Status::Won || active->status() == Status::Lost)) {
        uint64_t animation =
            finished_since ? std::min<uint64_t>(1400, (previous_us - finished_since) / 1000) : 0;
        canvas.rect(350, 235, 580, 180, color::Panel, 12, uint8_t(160 + animation * 75 / 1400));
        canvas.text(405, 265,
                    active->status() == Status::Won ? "PARTIDA CONCLUIDA!" : "FIM DE JOGO",
                    color::Gold, Font::Title);
        canvas.text(405, 330, "Preparando seu recorde...", color::Text);
        canvas.rect(405, 380, int(470 * animation / 1400), 4, color::Gold, 2);
    }
}
bool Application::save_active() {
    if (!active)
        return false;
    bool saved = journal.write(save_key(selected_game), 3, active->save());
    notice =
        saved ? "Partida salva. Voce pode continuar depois."
              : "Falha ao salvar. NVS indisponivel ou sem espaco; a partida continua na memoria.";
    return saved;
}
void Application::pause_game() {
    active->pause();
    save_active();
    pending = {};
    show(Screen::Pause);
}
void Application::name_editor(bool record) {
    naming_record = record;
    std::strncpy(name, settings.player, 15);
    name[15] = 0;
    show(Screen::Name, 1);
}
void Application::submit_name() {
    if (!name[0])
        std::strcpy(name, "Jogador");
    if (!naming_record) {
        std::strncpy(settings.player, name, 15);
        settings.player[15] = 0;
        persist_settings();
        show(Screen::Settings);
        return;
    }
    auto result = active->result();
    Record record{};
    std::strncpy(record.player, name, 15);
    record.score = result.score;
    record.time_ms = result.time_ms;
    record.distance = result.distance;
    record.level = result.level;
    record.won = result.won;
    std::time_t now = std::time(nullptr);
    if (now >= 1767225600)
        record.date = uint32_t(now);
    if (records.add(registry()[selected_game].id, options.mode, options.difficulty, record)) {
        journal.erase(save_key(selected_game));
        notice = "Partida registrada nesta categoria.";
        record_pending = false;
        active->cleanup();
        active = nullptr;
        show(Screen::Records);
    } else {
        notice =
            "Nao foi possivel gravar o recorde. Libere espaco no armazenamento e tente novamente.";
        show(Screen::Name, 1);
    }
}
void Application::navigation(uint32_t now) {
    if (current == Screen::Pause && input.pressed(binding(Binding::Pause))) {
        active->resume();
        show(Screen::Playing);
        return;
    }
    if (current == Screen::Name) {
        bool changed = false;
        for (unsigned code = 1; code < 256; ++code)
            if (input.pressed(uint8_t(code))) {
                char character = key_character(uint8_t(code), input.held(0xe1));
                size_t length = std::strlen(name);
                if (character && length < 15) {
                    name[length] = character;
                    name[length + 1] = 0;
                    changed = true;
                }
            }
        if (input.repeat(key::Backspace, now) && name[0]) {
            name[std::strlen(name) - 1] = 0;
            changed = true;
        }
        if (input.repeat(binding(Binding::Up), now))
            menu.select(1);
        if (input.repeat(binding(Binding::Down), now))
            menu.select(2);
        if (input.pressed(binding(Binding::Confirm))) {
            if (menu.selection() == 2)
                menu.activate();
            else
                submit_name();
            return;
        }
        if (input.pressed(binding(Binding::Back))) {
            menu.select(2);
            menu.activate();
            return;
        }
        if (changed)
            show(Screen::Name, 1);
        return;
    }
    if (current == Screen::Remap && remap >= 0) {
        for (unsigned code = 1; code < 256; ++code)
            if (input.pressed(uint8_t(code))) {
                if (code == key::Escape) {
                    remap = -1;
                    show(Screen::Remap);
                    return;
                }
                bool duplicate = false;
                for (unsigned i = 0; i < size_t(Binding::Count); ++i)
                    if (int(i) != remap && settings.bindings[i] == code)
                        duplicate = true;
                if (duplicate) {
                    notice = "Esta tecla ja esta em uso. Escolha outra.";
                    show(Screen::Remap, unsigned(remap));
                    return;
                }
                settings.bindings[remap] = uint8_t(code);
                unsigned selection = unsigned(remap);
                remap = -1;
                persist_settings();
                show(Screen::Remap, selection);
                return;
            }
        return;
    }
    if (input.repeat(binding(Binding::Up), now) && menu.count()) {
        menu.select((menu.selection() + menu.count() - 1) % menu.count());
        audio::play(audio::Effect::Navigate);
    }
    if (input.repeat(binding(Binding::Down), now) && menu.count()) {
        menu.select((menu.selection() + 1) % menu.count());
        audio::play(audio::Effect::Navigate);
    }
    if (input.repeat(binding(Binding::Left), now))
        menu.adjust(-1);
    if (input.repeat(binding(Binding::Right), now))
        menu.adjust(1);
    if (input.pressed(binding(Binding::Confirm))) {
        menu.activate();
        return;
    }
    if (input.pressed(binding(Binding::Back))) {
        if (current == Screen::Pause) {
            active->resume();
            show(Screen::Playing);
        } else if (current == Screen::Confirm)
            show(return_screen);
        else if (current == Screen::Setup)
            show(Screen::Games);
        else if (current == Screen::Controls || current == Screen::Food)
            show(Screen::Setup);
        else if (current == Screen::Remap)
            show(Screen::Settings);
        else
            show(Screen::Home);
    }
}
void Application::handle_sounds() {
    uint32_t sounds = active->sounds();
    const uint32_t bits[] = {Move,  Rotate, Drop,     Line,  Combo,    Eat,  Special,
                             Bonus, Crash,  Overtake, Turbo, GameOver, Level};
    const audio::Effect effects[] = {
        audio::Effect::Move,     audio::Effect::Rotate, audio::Effect::Drop,
        audio::Effect::Line,     audio::Effect::Combo,  audio::Effect::Eat,
        audio::Effect::Special,  audio::Effect::Bonus,  audio::Effect::Crash,
        audio::Effect::Overtake, audio::Effect::Turbo,  audio::Effect::GameOver,
        audio::Effect::Level};
    for (unsigned i = 0; i < 13; ++i)
        if (sounds & bits[i])
            audio::play(effects[i]);
}
void Application::tick(uint64_t now_us) {
    uint32_t now = uint32_t(now_us / 1000);
    keyboard::drain(input, now);
    uint64_t delta = previous_us ? now_us - previous_us : 0;
    previous_us = now_us;
    if (current != Screen::Name && !(current == Screen::Remap && remap >= 0) &&
        input.pressed(binding(Binding::Mute))) {
        settings.mute = !settings.mute;
        persist_settings();
        if (current != Screen::Playing)
            show(current, menu.selection());
    }
    if (current != Screen::Playing) {
        navigation(now);
        input.end_frame();
        return;
    }
    if (active->status() == Status::Lost || active->status() == Status::Won) {
        audio::engine(0);
        if (!finished_since)
            finished_since = now_us;
        if (now_us - finished_since > 1400000 || input.pressed(binding(Binding::Confirm))) {
            record_pending = true;
            name_editor(true);
            input.end_frame();
            return;
        }
    } else {
        if (input.pressed(binding(Binding::Pause)) || input.pressed(binding(Binding::Back))) {
            pause_game();
            input.end_frame();
            return;
        }
        if (input.pressed(binding(Binding::Restart))) {
            active->pause();
            confirm(
                "Reiniciar a partida?",
                [this] {
                    active->reset();
                    show(Screen::Playing);
                },
                Screen::Pause);
            input.end_frame();
            return;
        }
        const Binding binds[] = {Binding::Left, Binding::Right,   Binding::Up,
                                 Binding::Down, Binding::Primary, Binding::CounterRotate,
                                 Binding::Hold};
        for (unsigned i = 0; i < size_t(Action::Count); ++i) {
            uint8_t code = binding(binds[i]);
            bool press = (registry()[selected_game].id == GameId::BlockDrop && i < 2)
                             ? input.repeat(code, now, 170, 45)
                             : input.pressed(code);
            if (i == size_t(Action::Up) && registry()[selected_game].id == GameId::BlockDrop &&
                std::find(std::begin(settings.bindings), std::end(settings.bindings), key::X) ==
                    std::end(settings.bindings))
                press |= input.pressed(key::X);
            pending.press[i] |= press;
            pending.held[i] = input.held(code);
        }
        accumulator += std::min(delta, uint64_t(100000));
        while (accumulator >= 16667 && active->status() == Status::Running) {
            step_fraction += 16667;
            uint32_t millis = step_fraction / 1000;
            step_fraction %= 1000;
            active->update(millis, pending);
            pending.press.fill(0);
            accumulator -= 16667;
        }
        handle_sounds();
        if (registry()[selected_game].id == GameId::Racer)
            audio::engine(static_cast<const racer::RacerGame *>(active)->model.state.speed);
    }
    if (now_us - last_frame >= 16667 && surface) {
        bool changed = true;
        if (registry()[selected_game].id == GameId::BlockDrop &&
            active->status() == Status::Running) {
            // Falling-block visuals change on input, a cell step, clears or HUD
            // seconds. Keep invisible gravity/lock clocks out of redraw decisions.
            block::State visible;
            const auto &state = static_cast<const block::BlockGame *>(active)->model.state;
            std::memcpy(&visible, &state, sizeof(visible));
            visible.gravity = visible.lock = 0;
            visible.elapsed /= 1000;
            visible.clear_ms /= 35;
            visible.feedback_ms = visible.feedback_ms ? 1 : 0;
            uint32_t crc = crc32(reinterpret_cast<const uint8_t *>(&visible), sizeof(visible));
            changed = !visual_valid || crc != visual_crc;
            visual_crc = crc;
            visual_valid = true;
        }
        if (changed)
            lv_obj_invalidate(surface);
        last_frame = now_us - (now_us - last_frame) % 16667;
    }
    if (settings.fps && now_us - fps_since >= 1000000) {
        uint32_t frames = display::refreshes();
        fps = float(frames - frame_count) * 1000000.f / float(now_us - fps_since);
        frame_count = frames;
        fps_since = now_us;
        if (fps_label)
            lv_label_set_text_fmt(fps_label, "Render: %.1f FPS", double(fps));
    }
    input.end_frame();
}
} // namespace arcade
