# Frogger

Implementação original, com formas programáticas e sons sintetizados. Modelo
independente do hardware em `main/games/frogger`, renderer separado, ID 6.

Campo 720×520: início na linha 12, margem na 11, cinco faixas de carros nas
linhas 6–10, margem na 5, quatro faixas de troncos nas linhas 1–4 e cinco
abrigos na linha 0. Setas saltam, com repetição ao manter pressionado a cada
140 ms e fila de uma ação. Visual interpola o salto por 80 ms; a regra de
aterrissagem considera o destino. Troncos carregam o sapo continuamente.

Carro, água, bordas do rio, abrigo ocupado ou fim do tempo custam uma vida.
Há três vidas e 700 ms de retorno após erro. Abrigos conquistados permanecem.
Uma linha mais alta dá 10 pontos por tentativa; chegada vale 200 mais tempo
restante/100; cinco chegadas dão mais 1000 pontos.

Tempo base: 45/40/35/30 s, conforme dificuldade, reduzido por rodada até o
piso de 20 s. Chegada e nova tentativa renovam o tempo. Dificuldade e rodadas
aumentam a velocidade das faixas, que alternam direção, comprimento e posição.

| Modo | Objetivo |
|---|---|
| Campanha | Três rodadas, quinze chegadas |
| Arcade | Rodadas contínuas |
| Desafio | Pontuar em três minutos, sem perder as vidas |
| Treino | Vidas ilimitadas, com obstáculos e relógio |

Simulação fixa de 10 ms na tarefa de regras, sem alocar objetos por quadro.
Save completo em `/pipoca5/save6a.bin` e `save6b.bin`: faixas, sapo, salto,
fila de entrada, vidas, abrigos, rodada, RNG e tempos. TOP10 por modo e
dificuldade usa o módulo existente. P / Esc pausa e salva; R pede reinício.

Testes cobrem fila, carros, água, troncos/transporte, bordas, abrigos ocupados,
progressão, vitória, relógio, modos e saves inválidos/determinísticos. Há 200 s
de simulação com restaurações periódicas, além de integração LVGL e FatFs.
Prévia: `docs/previews/frogger.png`. Controles, áudio e desempenho do novo
jogo ainda precisam de validação no Tab5.
