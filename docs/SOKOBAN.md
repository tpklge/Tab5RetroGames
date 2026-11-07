# Sokoban

Jogo original de empurrar caixas. Módulo em `main/games/sokoban`, ID 9.
Contém 80 fases originais em quatro pacotes de dificuldade, com 20
fases por pacote e uma, duas, três ou quatro caixas conforme dificuldade.

## Controles e regras

- Setas: caminhar e empurrar uma caixa por vez; manter pressionado repete.
- Z: desfazer, com histórico das últimas 512 jogadas.
- X: reiniciar apenas a fase, preservando a campanha concluída e o tempo.
- R: reiniciar toda a partida, com confirmação pelo menu comum.
- P / Esc: pausar e salvar; M alterna som.

As caixas devem ocupar todos os círculos dourados. Não é possível puxar,
atravessar paredes ou empurrar duas caixas juntas. Caixas em alvos ficam
verdes. Um aviso identifica caixa presa em canto fora de alvo; não encerra
a partida, permitindo desfazer ou reiniciar. O detector cobre cantos simples,
não pretende detectar todos os tipos possíveis de bloqueio.

Cada passo válido conta um movimento. Um passo que empurra também conta um
empurrão. Desfazer restaura tabuleiro, posição e contadores. O tempo continua
contando durante as tentativas; desfazer não o reduz. Há uma janela de 1,2 s
após resolver, durante a qual Z ainda pode desfazer a conclusão.

## Modos e recordes

- Campanha: 20 fases do pacote selecionado.
- Limite de movimentos: limite baseado na solução de referência mais uma
  margem de 12/10/8/6 movimentos conforme dificuldade.
- Limite de empurrões: solução de referência mais uma margem de 3/3/2/2.
- Treino: repete o pacote, sem terminar ao concluir a vigésima fase.

Atingir um limite bloqueia ações que o excederiam, mantendo Z e X disponíveis.
No limite de empurrões ainda se pode caminhar sem empurrar. Não há derrota
automática que impeça recuperar uma tentativa.

A pontuação por fase é 10000 + dificuldade×250 - movimentos×15 - empurrões×35,
com mínimo de 100. Só é creditada ao confirmar a conclusão após a janela.
Nos recordes, campanhas concluídas vêm primeiro; menor quantidade total de
movimentos tem prioridade. No modo de empurrões, essa prioridade usa o total
de empurrões. O quadro mostra `mov`/`emp`, em vez de metros. Desfazer permite
melhorar a rota contada, e o tempo permanece como desempate posterior.

## Fases e persistência

`tools/generate_sokoban_levels.py` produz os mapas a partir de puxadas
reversas e encontra uma rota de solução usando busca por empurrões. Todas
as fases têm soluções verificadas; não se afirma que as referências tenham
mínimo global de movimentos. O catálogo está em `levels.h`; o solver não
roda durante o jogo nem bloqueia controles.

`/pipoca5/save9a.bin` e `save9b.bin` guardam caixas, jogador, fase, pacote,
contadores, campanha, relógios e o histórico completo de desfazer. Validação
confere o mapa original, contagem de caixas/alvos e reproduz o histórico em
sentido inverso numa cópia antes de aplicar o save. Nomes FAT 8.3 e CRC comuns.

As 80 soluções são executadas nos quatro modos, incluindo pontuação e
progressão. Testes cobrem paredes, empurrar/não puxar, cantos, desfazer 512
passos, reinício, limites, restauração e histórico inválido. Integração LVGL
testa setas, Z, X, desenho e pausa/save9 com desfazer após carregar.
O renderer evita atualizar quadros sem mudança visível no quebra-cabeça.
Prévia: `docs/previews/sokoban.png`. Controles, áudio e desempenho do novo
jogo ainda precisam ser experimentados no Tab5 físico.

As 16 fases anteriores foram preservadas nas mesmas posições de cada pacote.
Saves antigos concluídos na fase 4 continuam na fase 5; os demais retomam
normalmente. Recordes de campanhas com 20 fases precedem os antigos de quatro.
