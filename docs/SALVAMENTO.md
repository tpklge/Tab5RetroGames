# Salvamento no microSD

Insira um cartão FAT compatível antes de ligar o Tab5. O firmware monta o
cartão sem formatar e cria `pipoca5` na raiz. Essa pasta passa a armazenar
partidas, TOP10 e configurações, mesmo quando não há partição NVS.

| Jogo | Arquivos de partida em `/pipoca5` |
|---|---|
| Block Drop | `save0a.bin`, `save0b.bin` |
| Snake | `save1a.bin`, `save1b.bin` |
| Retro Racer | `save2a.bin`, `save2b.bin` |
| Space Invaders | `save3a.bin`, `save3b.bin` |
| Space Impact | `save4a.bin`, `save4b.bin` |
| Quebra-blocos | `save5a.bin`, `save5b.bin` |
| Frogger | `save6a.bin`, `save6b.bin` |
| Pac-Man | `save7a.bin`, `save7b.bin` |
| Bomberman | `save8a.bin`, `save8b.bin` |
| Sokoban | `save9a.bin`, `save9b.bin` |

Um save pode ter só um dos arquivos antes do segundo salvamento.
`configa.bin`/`configb.bin` guardam preferências. `rankGMDa.bin` e
`rankGMDb.bin` guardam TOP10, onde G é jogo, M modo e D dificuldade (IDs
numéricos iniciados em zero). Preserve os pares a/b ao copiar o cartão.

P / Esc pausa e salva automaticamente. O menu de pausa também permite
salvar manualmente ou voltar salvando. Continuar partida lê o estado completo,
incluindo RNG, chefes, projéteis, tabuleiro e efeitos próprios de cada jogo.
Não há escrita de save por quadro.

## Integridade e escolha do armazenamento

O journal alterna duas gerações, com versão, tamanho, tipo e CRC32. A
gravação vai primeiro para `.tmp`, faz flush/fsync, fecha e renomeia
para o slot mais antigo. O outro slot válido permanece disponível. Arquivos
temporários incompletos são ignorados; cada modelo valida o conteúdo antes
de restaurar. FAT/cartão continuam exigindo validação física de falha de
energia; o mecanismo não promete imunidade a corrupção do sistema de arquivos.

O meio é escolhido uma vez por inicialização: microSD montado com pasta
utilizável; senão NVS, caso disponível. O menu inicial e Configurações
mostram o destino. Se nenhum existir, Salvar tenta inicializar novamente o cartão antes de
consultar as gerações do save, permitindo recuperar essa situação sem
reiniciar ou perder a partida em RAM. A partida permanece na RAM após falha de save.

Não remova o cartão durante o uso. Trocar um meio já selecionado exige reinicializar o aplicativo. Se o boot
ficou sem nenhum meio, inserir o cartão e tentar Salvar novamente dispara
uma nova montagem. Erros de escrita no cartão são apresentados; não há mudança
silenciosa para NVS durante a sessão, evitando misturar gerações antigas e
novas. Dados anteriores do NVS não são migrados ou mesclados automaticamente;
continuam preservados e acessíveis ao iniciar sem cartão utilizável.

`Configurações → Backup no microSD` continua criando um arquivo complementar
`backup.bin`, seguido por nomes numerados, com registros do meio ativo. A
montagem é compartilhada com os saves e não é desmontada ao concluir o backup.
Backups não são importados automaticamente pelo arcade.

Testes nativos usam arquivos reais: restauração dos cinco modelos após
reabrir o armazenamento, recordes, configurações, CRC, arquivo truncado,
temporário incompleto, falha de substituição, exclusão e cartão removido.
Leitura/escrita via SDMMC ainda depende de teste no Tab5.

## Inicialização corrigida para o Tab5

A montagem usa **SDMMC slot 0**, GPIOs 43/44/39–42 e controle de alimentação
**LDO 4**, como no [exemplo oficial da M5Stack](https://github.com/m5stack/M5Tab5-UserDemo/blob/main/platforms/tab5/components/m5stack_tab5/m5stack_tab5.c).
O inicializador padrão ESP-IDF selecionava slot 1 e o código anterior não
associava o controle de alimentação SD; esses dois pontos foram corrigidos.
Frequência inicial de 20 MHz/4 linhas. Em timeout, resposta inválida ou CRC
inválido durante montagem, tenta 10 MHz/1 linha. Falha FAT não provoca
formatação nem essa troca de barramento.

Falhas de LDO ou montagem mostram o nome `ESP_ERR_*` na tela. Falhas nos
arquivos mostram etapa e erro do sistema: criar pasta, abrir, escrever,
flush, sincronizar, fechar, substituir ou renomear. O novo preparo do
backend acontece antes de ler as gerações do journal, evitando reiniciar a
numeração ao encontrar arquivos preexistentes em um cartão recém-inserido.

Testes de montagem usam o arquivo de produção com APIs de hardware simuladas.
Não substituem validar o cartão real após instalar o BIN no Tab5.

## Correção do erro 22 (nomes FAT 8.3)

O firmware usa `CONFIG_FATFS_LFN_NONE=y`: cada componente de arquivo deve
ter até oito caracteres no nome e três na extensão. O temporário anterior
`save0a.bin.tmp` viola esse formato; o FatFs retorna `FR_INVALID_NAME`,
traduzido pelo VFS para `EINVAL` (22). Agora usa `save0a.tmp`, mantendo
`save0a.bin` como destino. Saves a/b já existentes não mudam de nome ou formato.

O mesmo ajuste vale para todos os jogos, configurações e recordes. Backups
passam a usar `backup.bin`, `bak0001.bin`, `bak0002.bin` até `bak9999.bin`.
Arquivos anteriores são preservados. Não foi necessário remover `fsync`
ou ignorar erros de gravação para contornar a falha.

O teste nativo novo compila o FatFs original do ESP-IDF sobre um disco RAM,
com nomes longos desativados: reproduz os nomes antigos rejeitados e
confirma escrita/sync/rename/leitura com os nomes usados pelo firmware.
Esse disco RAM é formatado apenas pelo teste no computador; o firmware
continua sem formatar o cartão físico. Validação no Tab5 segue pendente.

Asteroids (ID 10) usa `save10a.bin`/`save10b.bin`. Para manter os rankings
em FAT 8.3, o identificador 10 é representado por A: `rankA00a.bin` até
`rankA33b.bin`. Os nomes existentes não mudam. Backup e leitor Python incluem
essas chaves. Sokoban preserva saves anteriores; campanhas concluídas de
quatro fases continuam na quinta das atuais vinte fases por dificuldade.
