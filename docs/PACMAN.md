# Pac-Man

Jogo de labirinto inspirado no clássico, com mapas, sprites programáticos e
implementação originais. Não usa ROMs, mapas ou assets comerciais. Modelo
em `main/games/pacman`, renderer separado, ID 7.

Três labirintos de 25×23 células, todos com conectividade verificada. Setas
selecionam a direção desejada; o jogador continua na direção atual até
conseguir virar. Curvas podem ser pedidas antes das esquinas. A viagem entre
células é interpolada; o túnel central liga as duas bordas.

Pontos valem 10; quatro orbes grandes valem 50 e concedem poder por 7/6/5/4 s,
conforme dificuldade. Capturar inimigos concede 200, 400, 800 e 1600 pontos,
reiniciando combo no próximo orbe. Eles reaparecem após dois segundos.
Frutas surgem após um e dois terços dos pontos, duram 9 s e valem 100 + 50×nível.
Limpar o mapa concede 1000×nível. Morte preserva os pontos coletados e reinicia
posições com 1,5 s de imunidade. Há três vidas, salvo no Treino.

| Inimigo | Comportamento |
|---|---|
| Rastro, vermelho | Persegue a posição atual |
| Rosa | Antecipa até quatro células na direção do jogador |
| Ciano | Patrulha cantos em sequência |
| Laranja | Persegue de longe e recua quando próximo |

Alternam 7 s de dispersão e 20 s de perseguição. Buscas de distância escolhem
caminhos, com desempate pelo RNG salvo. Evitam reversão havendo alternativa;
com poder, ficam lentos e fogem. Entrada no mapa é escalonada.

Clássico tem três mapas; Arcade os repete com progressão; Desafio dura três
minutos; Treino dá vidas ilimitadas. Quatro dificuldades alteram velocidade
e poder. P / Esc pausa e salva; R pede reinício; M alterna som.

Simulação de 10 ms na tarefa de regras, com snapshot para desenho. Paredes
usam trechos horizontais; os pontos normais usam faixas A8 reutilizáveis.
Na cena inicial, submissões de desenho passam de 517 para 192, sem equivaler
a uma medição de FPS físico. Cache de faixas é atualizado só pela UI.

Save completo em `/pipoca5/save7a.bin` e `save7b.bin`: paredes/pontos, viagem
dos atores, direção desejada, vidas, poder, frutas, combo, tempos, RNG e nível.
Validação confere limites, vizinhança, relógios, contagem e conectividade.
TOP10 por modo/dificuldade usa o armazenamento existente, com nomes FAT 8.3.

Testes cobrem mapas, curvas, interpolação, túnel, pontos/poder, alvos dos
inimigos, capturas/combo/dano, frutas, modos, vitória e determinismo. Há 200 s
de simulação com restaurações periódicas, integração LVGL/save7 e comparação
pixel a pixel das faixas A8, incluindo mudança de conteúdo após coleta.
Prévia: `docs/previews/pacman.png`. O novo jogo ainda requer validação de
controles, áudio e desempenho no Tab5.
