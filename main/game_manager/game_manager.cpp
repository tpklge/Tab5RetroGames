#include "game.h"
#include "games/block_drop/render.h"
#include "games/snake/render.h"
#include "games/retro_racer/render.h"
namespace arcade {
const std::vector<Descriptor> &registry() {
    static const char *block_modes[] = {"Maratona", "Sprint 40 linhas", "Ultra 2 minutos", "Zen"};
    static const char *snake_modes[] = {"Classico", "Sobrevivencia", "Desafio 2 minutos",
                                        "Labirinto"};
    static const char *racer_modes[] = {"Arcade", "Contra o relogio", "Transito intenso",
                                        "Estrada sem fim"};
    static const std::vector<Descriptor> games = {
        {GameId::BlockDrop, "Block Drop", "Encaixe pecas, crie combos e supere seus recordes.",
         block_modes,
         "Setas: mover / girar\nBaixo: queda suave\nX ou Cima: girar horario\nZ: girar "
         "anti-horario\nEspaco: queda instantanea\nC: reservar peca\nP / Esc: pausar",
         block::create},
        {GameId::Snake, "Snake", "Explore mapas, alimentos raros e efeitos especiais.", snake_modes,
         "Setas: mudar direcao\nNao e permitido inverter 180 graus.\nP / Esc: pausar\nR: solicitar "
         "reinicio\nM: ligar / desligar som",
         snake::create},
        {GameId::Racer, "Retro Racer", "Acelere, ultrapasse e domine as estradas.", racer_modes,
         "Esquerda / Direita: dirigir\nCima: acelerar\nBaixo: frear\nEspaco: turbo\nCombine "
         "direcao e aceleracao.\nP / Esc: pausar",
         racer::create}};
    return games;
}
} // namespace arcade
