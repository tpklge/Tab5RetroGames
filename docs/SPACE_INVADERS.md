# Space Invaders

Jogo original inspirado nos arcades de invasores e na experiência de jogos
de celular da época do Nokia N95. Não reproduz uma edição específica nem
utiliza código, imagens ou sons proprietários.

Disparo na tecla X (segure para repetir) e ícone próprio de invasor no menu.

Modelo independente de hardware em `main/games/space_invaders`, sprites
programáticos coloridos e campo horizontal no Tab5. Cinquenta invasores
marcham lateralmente e descem ao alcançar as bordas. A formação acelera
conforme perde integrantes. Somente o último invasor de cada coluna atira.
Escudos sofrem dano dos dois tipos de tiro e são destruídos por contato com
a formação. Três vidas, imunidade temporária após dano, nave bônus de 150
pontos e inimigos de 10/20/30 pontos por fileira.

Quatro modos: Arcade infinito; Campanha com cinco ondas; Desafio de dois
minutos; Tiro automático. Dificuldade aumenta a cadência e velocidade dos
tiros inimigos e a velocidade de marcha. Novas ondas restauram os escudos,
aumentam a dificuldade e começam mais próximas do jogador.

Simulação em passos de 10 ms, com acumulador salvo. Saves incluem RNG,
formação, projéteis, escudos, nave bônus, vidas e temporizadores, protegidos
pelo journal CRC existente. ID 3 preserva os IDs/saves dos jogos anteriores.
Recordes TOP10 por modo/dificuldade usam o sistema compartilhado. Efeitos
usam os eventos sintetizados existentes (tiro, acerto, dano, bônus e onda).

Testes nativos cobrem movimento/limites, pausa, colisões, escudos, imunidade,
bônus, avanço de onda, vitória, invasão, derrota por vidas, tiro automático,
validação de save e determinismo temporal. Integração LVGL cobre entrada
simultânea, renderização, pausa e save no namespace separado.
Validação física de controles, áudio e desempenho ainda depende do Tab5.
