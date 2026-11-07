# Quebra-blocos

Jogo original de plataforma e bola, inspirado em quebra-blocos de arcade.
Gráficos programáticos e efeitos sintetizados do Pipoca5, sem assets ou
código das versões comerciais. ID 5, acrescentado ao final do registro.
Os saves anteriores mantêm seus IDs e formatos.

## Jogabilidade

Setas laterais movem a plataforma; X lança a bola. O ponto de contato e o
movimento da plataforma definem o ângulo de saída. As velocidades têm
componente vertical mínima para evitar trajetórias quase horizontais
indefinidas. A bola rebate no topo e nas laterais; cair abaixo do campo
remove a bola. Só perde uma vida quando todas as bolas caem.

Três vidas; após perder vida, a bola volta à plataforma e aguarda novo
lançamento. A plataforma acompanha a tecla mantida e respeita os limites.
Não há ações automáticas que alterem a direção da bola sem colisão.

Blocos precisam de um a três impactos; marcas escuras indicam resistência
restante. Cada impacto dá 10 pontos; destruir um bloco dá mais 50 pontos
mais 5 por fileira contada a partir de baixo. Concluir fase dá 500×fase.
Itens coletados dão 100 pontos. A cada sete blocos destruídos cai um item,
alternando expansão, multibola e lentidão. Itens que não forem recolhidos
pela plataforma desaparecem ao sair do campo.

| Item | Efeito |
|---|---|
| E | Plataforma 60 px mais larga durante 15 s |
| M | Cria bolas adicionais até o limite de três |
| L | Reduz velocidade da bola para 72% durante 10 s |

Recolher novamente E ou L renova o temporizador, sem acumular amplitude.
Perder todas as bolas ou trocar de fase encerra esses efeitos.

## Fases e modos

Seis desenhos: Primeiro Contato, Constelação, Portais, Ziguezague, Diamante
e Fortaleza. Todos os blocos podem ser destruídos. Novas fases têm velocidade
maior; o teto é 630 px/s antes da lentidão. Quatro dificuldades alteram a
velocidade inicial (255/310/365/420 px/s) e a largura da plataforma
(140/118/102/90 px). Movimento lateral: 620 px/s.

- Campanha: vencer seis fases.
- Arcade: repetir os seis desenhos com dificuldade progressiva.
- Desafio: maximizar pontuação em três minutos; perder as vidas encerra antes.
- Treino: vidas ilimitadas, preservando colisões e necessidade de relançar.

## Integração

Modelo sem hardware em `main/games/breakout`, configuração e renderer
separados. Passos internos de 5 ms limitam deslocamento a 3,15 px na
velocidade máxima; colisão usa círculo/retângulo e reposicionamento fora
da superfície para evitar atravessamentos e impactos repetidos por
sobreposição. Simulação continua na tarefa independente; UI usa snapshot.

Pausa, reinício confirmado, recordes TOP10 por modo/dificuldade e save
completo usam os módulos existentes. `/pipoca5/save5a.bin` e `save5b.bin`
armazenam blocos, bolas, itens, plataforma, vidas, RNG, timers e fase.
Temporários continuam FAT 8.3. Não há gravação por quadro.

Testes nativos cobrem lançamento, limites, ângulos, paredes, impactos
laterais/verticais, resistência, itens, efeitos/expiração, multibola, vidas,
avanço/vitória, modo cronometrado/treino, determinismo, saves inválidos e
100 segundos de simulação em velocidade máxima com restaurações periódicas.
Integração LVGL cobre seleção do sexto jogo, X/movimento sem render,
desenho, pausa e save separado. FatFs real cobre saves e rankings do sexto
jogo. Build ESP32-P4 e pacotes OTA/USB gerados, sem flash ou serial.

Prévia usa o renderer LVGL real no computador, com bônus ativados como
cenário de teste. Controles, áudio e desempenho do novo jogo ainda precisam
de validação no Tab5.
