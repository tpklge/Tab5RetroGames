# Bomberman

Implementação original para o Pipoca5, com gráficos programáticos e efeitos
sintetizados. Módulo em `main/games/bomberman`, ID 8, preservando os IDs e
saves anteriores. Usa simulação de 10 ms e cópia do estado para desenho.

## Regras e controles

Setas movem o personagem; X coloca uma bomba. A bomba ocupa a casa atual:
é possível sair dela, mas não atravessá-la ou voltar enquanto estiver ativa.
A capacidade inicial é uma bomba e o alcance inicial é duas casas.

O pavio dura 2,2 s. A explosão propaga em cruz e dura 500 ms. Paredes sólidas
bloqueiam; a primeira caixa destrutível é removida e interrompe aquele raio.
Bombas atingidas explodem imediatamente, permitindo reação em cadeia.
Explosões também atingem inimigos e o jogador. Há três vidas; um erro mantém
caixas removidas e inimigos eliminados, limpa bombas/chamas e devolve o
jogador ao início com proteção temporária. O relógio da tentativa é renovado.

| Item escondido | Efeito |
|---|---|
| F | Alcance maior, até seis casas |
| B | Uma bomba adicional, até cinco |
| V | Movimento mais rápido, intervalo de 100 ms em vez de 140 ms |

Itens são coletados ao pisar na casa após o fogo apagar. Outra explosão
pode destruir um item já exposto. As melhorias persistem entre vidas e arenas.
A saída S fica escondida sob uma caixa no canto inferior direito. É preciso
revelá-la, eliminar todos os inimigos e entrar nela para avançar.

Patrulheiros vagam; outros inimigos priorizam continuar por corredores ou
se aproximar do jogador, evitando áreas ameaçadas por bombas quando houver
opção. Todos respeitam paredes, caixas e bombas. O movimento é por casas,
com interpolação visual. O espaço inicial tem rotas laterais de fuga.

## Modos, dificuldade e pontos

- Campanha: cinco arenas.
- Arcade: arenas contínuas, com progressão.
- Desafio: pontuar em três minutos, podendo perder por vidas antes.
- Treino: vidas ilimitadas, mantendo perigos e objetivos.

Dificuldade altera densidade de caixas, quantidade/velocidade de inimigos e
prazo da arena. Novas arenas variam a distribuição, aumentam o número de
inimigos até oito e apertam o prazo. O relógio tem piso de um minuto.
Caixa destruída vale 25 pontos; inimigo, 200; item, 100; saída concede
1000×arena + tempo restante/100.

## Persistência e validação

`/pipoca5/save8a.bin` e `save8b.bin` guardam bombas/pavios/alcances, chamas,
caixas, itens, inimigos, posição e fila de movimento, vidas, melhorias,
relógios, RNG e arena. Journal CRC e nomes FAT 8.3 existentes. Sem escrita
por quadro. P / Esc pausa/salva; R pede reinício da partida; M alterna som.

Testes cobrem bomba própria, colisões, pavio, paredes/caixas, reação em
cadeia, fogo/dano, itens e limites, ameaça de explosão, saída, progressão,
modos, saves inválidos e determinismo. Simulação de 200 s no Treino verifica
saves periódicos. Integração LVGL testa X, fuga da própria bomba, desenho
de explosão e pausa/save8. Prévia: `docs/previews/bomberman.png`.
Controles, áudio, balanceamento e desempenho do novo jogo ainda precisam
ser experimentados no Tab5 físico.
