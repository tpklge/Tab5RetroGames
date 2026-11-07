# Space Impact

Implementação original de shooter horizontal, inspirada nos jogos de celular
da época do N-Gage, com gráficos e efeitos programáticos próprios. Sem assets
ou código das versões comerciais. Modelo independente de hardware em
`main/games/space_impact`; integração pelo adaptador padrão e registro ID 4,
preservando os saves dos outros jogos.

## Mecânicas

Setas movem a nave em quatro direções. Movimento diagonal normalizado. X
dispara continuamente; W melhora a arma até três projéteis, + recupera vida
e B repõe bomba. Um item aparece a cada cinco inimigos eliminados, alternando
arma, vida e bomba. Z usa uma bomba: elimina inimigos comuns, limpa tiros
hostis e causa 18 de dano ao chefe. Limites: três vidas e cinco bombas.
Dano reduz a arma em um nível e concede 1,8 s de imunidade.

Inimigos seguem trajetórias onduladas, perseguem a nave ou usam armadura e
disparos direcionados. Cenário tem estrelas em parallax e silhuetas decorativas;
estas não colidem. Inimigos, tiros e chefe colidem com a nave.

Cada setor dura 28 s antes da chegada do chefe:

| Setor | Chefe | Padrão |
|---|---|---|
| Órbita Esmeralda | Sentinela | Tiro direcionado e leque de três tiros |
| Nebulosa Violeta | Aracníde | Oscilação maior e leque de cinco tiros |
| Fortaleza Solar | Núcleo Omega | Movimento horizontal adicional e leque de sete tiros |

Todos os chefes entram em fúria abaixo de metade da energia, aumentando a
cadência. Vida do chefe, frequência de inimigos e velocidade/cadência dos
tiros variam com dificuldade; sobrevivência aumenta a pressão por setor.
Chefes dão 1500 pontos por número de setor. Inimigos dão 60/90/120 pontos;
itens dão 100. Novo setor concede uma bomba até o limite.

Campanha e Tiro automático terminam após vencer três chefes. Sobrevivência
repete os três ambientes. Desafio dura três minutos e também pode terminar
por derrota antes do prazo. Todos têm quatro dificuldades, pausa, save
completo e TOP10 separado por modo/dificuldade.

## Persistência e validação

Simulação em passos de 10 ms. Save inclui inimigos, projéteis, itens, RNG,
posição, arma, bombas, chefe, setor, pontuação e temporizadores; journal
existente protege o registro com versão/CRC. O modelo rejeita estados fora
dos limites sem alterar a partida atual. Arrays fixos limitam alocações
durante a simulação.

Testes nativos ASan/UBSan cobrem movimento, colisões, armas, itens, bomba
consumida uma vez por pressão, vidas/imunidade, três padrões de chefes,
progressão, modos, limites de entidades e continuidade determinística.
Integração LVGL cobre entrada simultânea, ícone/menu, disparo X, bomba Z,
renderização do chefe e save separado `save4`. Compilação real ESP32-P4
produz aplicação OTA e pacote USB, sem gravar ou abrir a serial.

Controles, áudio e desempenho ainda dependem de validação física no Tab5.
