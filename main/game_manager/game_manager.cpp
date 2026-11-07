#include "game.h"
#include "games/block_drop/render.h"
#include "games/snake/render.h"
#include "games/retro_racer/render.h"
#include "games/space_invaders/render.h"
#include "games/space_impact/render.h"
#include "games/breakout/render.h"
#include "games/frogger/render.h"
#include "games/pacman/render.h"
#include "games/bomberman/render.h"
#include "games/sokoban/render.h"
#include "games/asteroids/render.h"
namespace arcade {
const std::vector<Descriptor> &registry() {
    static const char *block_modes[] = {"Maratona", "Sprint 40 linhas", "Ultra 2 minutos", "Zen"};
    static const char *snake_modes[] = {"Classico", "Sobrevivencia", "Desafio 2 minutos",
                                        "Labirinto"};
    static const char *racer_modes[] = {"Arcade", "Contra o relogio", "Transito intenso",
                                        "Estrada sem fim"};
    static const char *invader_modes[] = {"Arcade", "Campanha 5 ondas", "Desafio 2 minutos", "Tiro automatico"};
    static const char *impact_modes[] = {"Campanha 3 setores", "Sobrevivencia", "Desafio 3 minutos", "Tiro automatico"};
    static const char *breakout_modes[] = {"Campanha 6 fases", "Arcade", "Desafio 3 minutos", "Treino"};
    static const char *frogger_modes[] = {"Campanha 3 rodadas", "Arcade", "Desafio 3 minutos", "Treino"};
    static const char *pacman_modes[] = {"Classico 3 labirintos", "Arcade", "Desafio 3 minutos", "Treino"};
    static const char *bomber_modes[] = {"Campanha 5 arenas", "Arcade", "Desafio 3 minutos", "Treino"};
    static const char *sokoban_modes[] = {"Campanha", "Limite de movimentos", "Limite de empurroes", "Treino"};
    static const char *asteroid_modes[] = {"Campanha 5 ondas", "Arcade", "Desafio 3 minutos", "Treino"};
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
         racer::create},
        {GameId::SpaceInvaders, "Space Invaders", "Defenda a Terra contra ondas de invasores espaciais.", invader_modes,
         "Esquerda / Direita: mover\nX: atirar (segure para repetir)\nProteja-se com os escudos.\nCampanha: sobreviva a 5 ondas.\nP / Esc: pausar",
         invaders::create},
        {GameId::SpaceImpact, "Space Impact", "Explore setores coloridos, melhore sua nave e enfrente chefes.", impact_modes,
         "Setas: mover em 4 direcoes\nX: atirar (segure)\nZ: bomba / limpa tiros inimigos\nW: melhora arma / +: vida / B: bomba\nP / Esc: pausar",
         impact::create},
        {GameId::Breakout, "Quebra-blocos", "Rebata, destrua blocos resistentes e colete multibola.", breakout_modes,
         "Esquerda / Direita: mover plataforma\nX: lancar bola\nRebata pelas bordas para mudar o angulo.\nE: expansao / M: multibola / L: lentidao\nCampanha: complete 6 fases.\nP / Esc: pausar",
         breakout::create},
        {GameId::Frogger, "Frogger", "Atravesse o transito, viaje nos troncos e alcance os cinco abrigos.", frogger_modes,
         "Setas: saltar / segure para repetir\nTroncos carregam o sapo.\nEvite carros, agua e abrigos ocupados.\nChegue rapido para ganhar bonus.\nP / Esc: pausar",
         frogger::create},
        {GameId::Pacman, "Pac-Man", "Colete pontos, use poderes e enfrente quatro perseguidores diferentes.", pacman_modes,
         "Setas: mover / antecipar curvas\nOrbes grandes: poder temporario\nCom poder: capture os inimigos.\nFrutas: pontos extras\nTunel lateral: atravessar as bordas\nP / Esc: pausar",
         pacman::create},
        {GameId::Bomberman, "Bomberman", "Abra caminhos, encadeie explosoes e descubra a saida escondida.", bomber_modes,
         "Setas: mover\nX: colocar bomba\nSaia da cruz antes da explosao!\nF: alcance / B: bombas / V: velocidade\nElimine os inimigos e procure a saida S.\nP / Esc: pausar",
         bomberman::create},
        {GameId::Sokoban, "Sokoban", "80 fases: 20 por dificuldade, com desfazer e busca pela melhor rota.", sokoban_modes,
         "Setas: mover / empurrar\nZ: desfazer (ate 512 passos)\nX: reiniciar somente a fase\nR: reiniciar campanha com confirmacao\nNao e possivel puxar caixas.\nP / Esc: pausar",
         sokoban::create},
        {GameId::Asteroids, "Asteroids", "Pilote com inercia, divida asteroides e enfrente discos voadores.", asteroid_modes,
         "Esquerda / Direita: girar\nCima: impulso / Baixo: frear\nX: disparar (segure)\nZ: hiperespaco (recarga de 8 s)\nNave, rochas e tiros atravessam as bordas.\nP / Esc: pausar",
         asteroids::create}};
    return games;
}
} // namespace arcade
