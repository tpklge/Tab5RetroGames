# Asteroids

Jogo original em `main/games/asteroids`, ID 10, com desenho vetorial colorido.

Esquerda/direita giram a nave; cima acelera; baixo freia. X dispara, inclusive
mantido. Z usa hiperespaço, com recarga de oito segundos e proteção breve.
P/Esc pausa e salva; R abre a confirmação de reinício; M alterna o som.

A nave mantém inércia e reaparece na borda oposta. Colisões e projéteis também
consideram essa continuidade. Asteroides grandes se dividem em dois médios,
e cada médio em dois pequenos. Discos voadores aparecem periodicamente e
atiram mirando a nave. São três vidas, com proteção ao reaparecer.

Modos: Campanha de cinco ondas, Arcade, Desafio de três minutos e Treino
sem perda de vidas. A dificuldade aumenta velocidade dos asteroides e
frequência/agressividade dos discos. Cada onda concluída concede bônus.

A simulação usa subpassos de 5 ms, limites de velocidade e vetores de tamanho
fixo para rochas e tiros. Não aloca objetos por disparo. Linhas são submetidas
ao LVGL como primitivas de desenho, com recorte nas bordas do campo.

Saves `/pipoca5/save10a.bin` e `save10b.bin` preservam nave, inércia,
projéteis, asteroides, disco, relógios e gerador aleatório. O carregamento
valida limites, números finitos e capacidade antes de aplicar o estado.
Rankings usam `rankA00a.bin` a `rankA33b.bin`, preservando FAT 8.3. As chaves
numéricas dos dez jogos anteriores permanecem iguais. Backup e leitor Python
aceitam as novas chaves.

Testes nativos cobrem física, bordas, colisões, divisões até a capacidade
máxima, tiros, disco, vidas, hiperespaço, modos e saves determinísticos.
A integração usa o LVGL de produção, navega pelo menu, gira/acelera/atira,
ativa hiperespaço e pausa para salvar/carregar. Prévia em
`docs/previews/asteroids.png`. Validação física no Tab5 ainda pendente.
