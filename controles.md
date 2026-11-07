# Controles

Teclado oficial: I²C 0x6D, SDA0/SCL1, INT50 sem ISR. Polling configurado em 4 ms em
modo **Normal**, com pressão/liberação na matriz. Códigos HID internos são
identificadores; o acessório não opera como USB.

| Tecla padrão | Menu / ação comum |
|---|---|
| Cima / Baixo | Escolher item com repetição |
| Esquerda / Direita | Ajustar valor selecionado |
| Enter | Ativar item / confirmar nome |
| Esc | Voltar / pausar / continuar da pausa |
| P | Pausar ou continuar |
| R | Reiniciar com confirmação, inicialmente em Cancelar |
| M | Alternar mudo; na edição de nome digita M |
| Backspace | Apagar caractere do nome |
| Shift + letra | Maiúscula |

| Jogo | Controles |
|---|---|
| Block Drop | Esquerda/Direita move; Baixo soft drop; Cima/X gira horário; Z anti-horário; Espaço hard drop; C hold |
| Snake | Setas mudam direção; fila de duas curvas, sem reversão direta de 180° |
| Retro Racer | Esquerda/Direita dirige; Cima acelera; Baixo freia; Espaço turbo. Direção/aceleração/turbo simultâneos |

`Configurações → Mapeamento de controles` altera doze ações e rejeita teclas
já atribuídas. Esc físico cancela a captura. X no Block Drop só é alias
quando não estiver atribuído a outra ação. Backspace, Shift e caracteres
do nome são teclas de edição fixas.

O nome começa com o nome padrão. São aceitos letras ASCII, dígitos, espaço,
hífen e ponto, até 15 caracteres. Enter registra; Esc volta, com confirmação
caso descarte um resultado terminado.

Eventos duplicados são rejeitados. Repetição do menu: espera 250 ms,
intervalo 65 ms. Lateral dos blocos: 170/45 ms. Fila de 128 eventos e estado
simultâneo preservam combinações; overflow libera entradas e limpa o FIFO.
No encerramento normal o teclado volta ao modo Normal.

Toque complementa menus. Não há joystick de tela. Teclado ausente no boot
permite usar menus pelo toque, mas os jogos precisam do acessório. Conecte-o
antes do boot; reconexão automática ainda não é implementada.

## Space Invaders

- Esquerda / Direita: movimentar nave.
- X: atirar; manter pressionado repete os disparos.
- P / Esc: pausar e acessar salvamento.
- R: solicitar reinício.
- M: alternar som.

Arcade tem ondas contínuas. Campanha termina após cinco ondas. Desafio dura
dois minutos, preservando a condição de derrota por vidas/invasão. Tiro
automático dispara sem pressionar X. Todos têm quatro dificuldades.

## Space Impact

- Setas: mover em quatro direções, inclusive diagonal.
- X: atirar; segure para disparar continuamente.
- Z: bomba; elimina inimigos comuns, limpa projéteis hostis e causa dano ao chefe.
- P / Esc: pausar e salvar. R solicita reinício; M alterna som.
- Colete W para melhorar a arma (três níveis), + para recuperar vida e B para repor bomba.

Campanha e Tiro automático terminam após três setores e seus chefes.
Sobrevivência repete os setores com progressão; Desafio dura três minutos.

Nos dois jogos espaciais, X substitui o padrão Espaço da ação principal.
Uma tecla personalizada para a ação principal continua sendo respeitada.
A bomba usa a ação configurável de rotação anti-horária (Z por padrão).

## Resposta e diagnóstico

A tarefa de entrada/regras roda separada do LVGL, com sondagem configurada
em 4 ms e simulação em passos de 10 ms. Renderização usa cópia do estado,
sem manter a trava da simulação enquanto desenha. Toques curtos de direção
são aplicados por pelo menos um passo; soltar encerra o movimento contínuo.
Snake continua respeitando a grade e sua fila de curvas.

P / Esc suspende as regras assim que o evento é consumido; o menu e o save
são processados pela tarefa de interface. Ative Mostrar FPS medido para ver
`Render ... FPS | Regras ... Hz`. O alvo configurado das regras é 100 Hz;
esses valores na tela são medidos, não garantias de FPS ou latência física.

## Quebra-blocos

- Esquerda / Direita: mover a plataforma.
- X: lançar a bola inicial ou após perder uma vida.
- P / Esc: pausar e salvar. R: reinício com confirmação; M: alternar som.
- Rebater próximo das bordas inclina a bola; mover a plataforma também influencia o ângulo.
- Colete E: expansão por 15 s; M: multibola até três bolas; L: lentidão por 10 s.
  As letras identificam os itens, que são recolhidos pela plataforma.

A vida só é perdida quando todas as bolas caem. Depois disso, X lança
novamente. Treino permite tentativas ilimitadas; Campanha termina após seis
fases; Arcade continua com velocidade progressiva; Desafio dura três minutos.
X é o padrão confortável da ação principal nesse jogo, como nos Spaces.

## Frogger

- Setas: saltar; manter pressionado repete a cada 140 ms.
- P / Esc: pausar/save; R pede reinício; M alterna som.
- Evite carros, água e abrigos ocupados; troncos carregam o sapo.
- Complete cinco chegadas por rodada. Chegada rápida ganha bônus.

Campanha tem três rodadas; Arcade continua; Desafio dura três minutos;
Treino tem vidas ilimitadas, mantendo obstáculos e relógio por tentativa.

## Pac-Man

- Setas: selecionar direção e antecipar curvas antes das esquinas.
- Orbes grandes: capturar inimigos por tempo limitado. Frutas dão bônus.
- O túnel liga as bordas; colete todos os pontos para avançar.
- P / Esc: pausa/save; R pede reinício; M alterna som.

Clássico tem três mapas; Arcade os repete; Desafio dura três minutos;
Treino tem vidas ilimitadas. Quatro dificuldades alteram velocidade e poder.

## Bomberman

- Setas: mover; uma pressão durante o intervalo fica em fila.
- X: colocar bomba com pavio de 2,2 s. Saia da cruz antes da explosão.
- F/B/V são itens coletáveis de alcance, capacidade e velocidade.
- Elimine os inimigos, revele a saída S sob uma caixa e entre nela.
- P / Esc pausa/salva; R pede reinício; M alterna som.

Campanha tem cinco arenas; Arcade continua; Desafio dura três minutos;
Treino tem vidas ilimitadas. Paredes e caixas bloqueiam a explosão; bombas
atingidas provocam reação em cadeia.

## Sokoban

- Setas: caminhar/empurrar; não é possível puxar ou empurrar duas caixas.
- Z: desfazer até 512 jogadas, inclusive depois de carregar uma partida.
- X: reiniciar só a fase, preservando as fases anteriores concluídas.
- R: pedir reinício da partida toda, com confirmação.
- P / Esc: pausa/save; M alterna som.

Há quatro pacotes de 20 fases, selecionados pela dificuldade. Atingir
um limite não encerra a tentativa: use Z ou X. No modo de empurrões, caminhar
ainda é permitido quando só novos empurrões atingiram o limite.

## Asteroids

Setas esquerda/direita giram; cima acelera; baixo freia. X atira (pode manter
pressionado); Z ativa hiperespaço com recarga de oito segundos. P/Esc pausa e
salva. R reinicia com confirmação. A nave mantém inércia ao soltar o acelerador.
