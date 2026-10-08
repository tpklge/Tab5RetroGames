# Adicionar um jogo

1. Crie `main/games/nome/` com `config.h`, `model.h/.cpp`, `render.h/.cpp`.
   Modelo sem ESP-IDF/LVGL/periféricos: recebe `Options`, `Controls`, delta em
   milissegundos; publica `Result` e eventos `Sound`.
2. Implemente `Game` (`game_manager/game.h`). `ModelGame<Model>` adapta ciclo,
   pausa, save e limpeza de modelos compatíveis. Implemente `render(Canvas&)`
   e fábrica de instância estática, como nos jogos existentes.
3. Acrescente ID ao final de `GameId`, sem renumerar os anteriores. Registre
   metadados, quatro modos, controles e fábrica em `game_manager.cpp`. Menus
   enumeram o registro, sem contagem fixa de três jogos no fluxo principal.
4. Rode `idf.py reconfigure build`; CMake agrega os `.cpp` automaticamente.
   Não usa `CONFIGURE_DEPENDS`, incompatível com a análise ESP-IDF em modo script.
5. Acrescente testes de colisão, fim, pontuação e continuidade determinística
   em `tests/run_tests.py`. Integração agrega renderers reais dos jogos.

Um quarto jogo com quatro modos/dificuldades usa as telas comuns. Opções
próprias podem exigir pequeno editor, como mapa/alimentos do Snake ou carros
do Racer. Novos módulos usam os nomes de dificuldade da última linha da tabela
atual; ajuste metadados se precisar de rótulos diferentes.

O estado persistido deve conter apenas dados triviais, tipos de largura fixa,
floats de 32 bits, arrays e enums dimensionados. Não inclua ponteiros,
`std::string`, vetores ou `size_t`. Inicialize padding, valide índices e
preserve RNG/temporizadores. Formato atual little-endian/layout ESP32-P4;
mudanças em uma versão publicada precisam de novo schema e migração ou
rejeição explícita de saves anteriores.

Core cuida de saves/TOP10. Modelos recebem ações lógicas; o desenho não altera
a simulação. `cleanup()` limpa estado/libera recursos ao trocar de jogo.
