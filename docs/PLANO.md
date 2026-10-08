# Plano e estado da entrega

| Etapa | Entrega | Estado |
|---|---|---|
| Pesquisa | Hardware, protocolo Normal, BSP, Launcher e contratos | Concluída e documentada |
| 1 — Plataforma | Landscape/PPA, teclado, áudio assíncrono | Implementada e compilada; validação física pendente |
| 2 — Menu | Teclado/toque, seleção, opções, confirmação e controles | Integração LVGL nativa passou |
| 3 — Block Drop | SRS, 7-bag, hold, spins, placar e quatro modos | Implementado; regras testadas |
| 4 — Snake | Quatro mapas/modos, seis alimentos/efeitos e saves | Implementado; regras testadas |
| 5 — Retro Racer | Veículos, física 2D, tráfego e quatro modos | Implementado; regras testadas |
| 6 — Persistência | Saves completos, preferências, TOP10, CRC/duas gerações | Testes passaram; capacidade NVS instalada depende do Launcher |
| 7 — Polimento | Música original, feedback, interpolação, temas e FPS | Implementado; áudio/FPS físicos pendentes |
| 8 — Pacotes | Build real, app OTA, imagem USB e documentação | Gerados; sem instalar no aparelho |

Os critérios de aceitação física de REQUISITOS.md ainda não estão concluídos.
Consultar TESTES.md e LIMITACOES.md. Não inferir boot, 60 FPS ou ausência de
tearing a partir de testes no computador ou compilação.
