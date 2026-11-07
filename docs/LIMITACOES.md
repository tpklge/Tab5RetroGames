# Limitações e aceitação física

Código e build disponíveis; a aceitação final de `REQUISITOS.md` exige testes
no Tab5. Nenhum periférico foi validado fisicamente nesta etapa.

| Item | Estado |
|---|---|
| ESP32-P4/ESP-IDF 5.4.4 | Build real passou; BINs OTA/USB gerados |
| Regras, entrada, journal, menus | 17 executáveis nativos com ASan/UBSan passaram |
| LCD/touch | BSP autodetecta ILI9881+GT911, ST7123/ST7121; confirmar etiqueta/log no aparelho |
| Teclado A164 | Normal pesquisado; estado simultâneo testado em software; polling físico pendente |
| Áudio | ES8388/I²S e síntese implementados; alto-falante/volume/ruído pendentes |
| Launcher | BIN válido abaixo do teto conservador; instalação/boot pendentes |
| FPS/tearing | Contador de render LVGL implementado; desempenho físico sem medição |
| microSD | Saves/TOP10/configurações e backup FAT/SDMMC compilados; escrita física pendente |
| Persistência sem cartão | Duas gerações NVS; capacidade depende da tabela instalada |

Checklist no aparelho:

- Instalar app OTA, iniciar a frio e verificar landscape/variante LCD.
- Navegar pelo teclado; manter aceleração+direção+turbo e soltar cada tecla.
- Jogar cada modo/dificuldade; conferir kicks, hold, spins, ghost, lock delay.
- Conferir alimentos/efeitos, wrap, obstáculos e colisões do Snake; timer 2 min.
- Conferir cinco carros, tráfego, colisões/imunidade, turbo e meta 3 km/60 s.
- Salvar os cinco jogos, reiniciar e continuar com estado/RNG/efeitos intactos.
- Registrar nomes/TOP10, alterar preferências, reiniciar e verificar dados.
- Testar som/mudo e backup FAT com/sem cartão.
- Observar FPS por 60 s em cada jogo. Contador mede renders LVGL concluídos,
  não scanouts do painel. Block Drop evita redraws quando nada visível muda;
  FPS baixo em trechos estáticos não representa a taxa da simulação.

Limites explícitos:

- NVS de 24 KiB pode esgotar com várias categorias; standalone prevê 128 KiB.
  Falhas aparecem sem apagar dados ou mudar partições. microSD agora é o
  meio principal quando disponível, independente do tamanho/existência do NVS.
- O cartão deve estar inserido no boot; não há troca de meio durante a partida.
- Backup é exportação complementar, sem importação no arcade nesta versão.
  `read_backup.py` inspeciona/extrai registros no computador.
- Conecte teclado antes do boot; não há reconexão automática.
- Repertório inicial ASCII: UI em português sem acentos; nomes até 15 caracteres,
  sem teclado virtual. Textos de controles indicam o padrão; remapeamentos são
  apresentados no menu de configuração.
- Snapshots seguem structs fixos ESP32-P4, com schema/tamanho/CRC. Não são
  portáveis para qualquer arquitetura. Mudanças futuras exigem migração/schema.
- Recuperação de atraso limitada a 100 ms/tick; atrasos maiores desaceleram
  o tempo simulado. Renderização precisa de medição física.
- Modelos não alocam por frame; LVGL gerencia tarefas/cópias de texto. Menus,
  saves e backup alocam ao executar; não se afirma zero alocação do renderer.
- Data depende de RTC existente válido (ano ≥2026); senão é zero. Sem NTP/rede.
- USB standalone substitui esquema do Launcher; OTA usa tabela existente.

Otimizações de fluidez foram compiladas e comparadas em software: fundo do
Snake reutilizado, máscaras dos invasores agrupadas, invalidação de campo e
rotação PPA habilitada. Resultado físico e estabilidade do PPA exigem medição
no Tab5. Não foi realizada gravação/abertura de serial.

A redução anterior de desenhos não trouxe ganho de FPS segundo o teste
do usuário. A revisão atual separa input/regras do LVGL, mantém cópia
estável para desenho e corrige toques curtos perdidos. Testes de software
cobrem UI sem atualização e restauração dos cinco jogos; ganho físico de
resposta/FPS ainda precisa ser confirmado. O contador mostra separadamente
Render FPS e Regras Hz para auxiliar essa verificação.

Montagem microSD corrigida para SDMMC0/LDO4. A versão anterior estava
incompleta nessa configuração física, apesar de os testes de arquivos
passarem. O novo teste de montagem simula as APIs de hardware; o cartão
real ainda requer confirmação no Tab5. Erros agora mostram códigos/etapas.

Erro de nome FAT 8.3 corrigido: temporários usam `chave.tmp`, sem dupla
extensão. O FatFs real, com LFN desativado, reproduz a rejeição dos nomes
antigos e aceita os novos em disco RAM. Confirmação no cartão físico ainda
é necessária; saves finais permanecem com os mesmos nomes e formato.

Quebra-blocos acrescentado como sexto jogo: regras, integração e save em
arquivos passaram; a aceitação física no Tab5 permanece pendente para
controles, áudio, fluidez e sensação dos rebotes.

Frogger e Pac-Man ampliam a coleção para oito jogos. Modelos, saves e
integração LVGL passaram nos testes. As prévias vêm do renderer no computador;
jogabilidade e desempenho dos novos jogos precisam de validação no Tab5.

Bomberman e Sokoban ampliam a coleção para dez jogos. Os testes de regras,
integração e arquivos passaram, inclusive as soluções das 16 fases do
Sokoban. Equilíbrio/jogabilidade dos novos jogos e resposta dos controles
ainda dependem da experiência no Tab5. O detector de bloqueios do Sokoban
identifica cantos simples; Z e X continuam disponíveis em outras situações.

Asteroids e a expansão do Sokoban passaram nos testes nativos e na compilação
ESP32-P4. Asteroids ainda depende de validação física de controles, áudio e
desempenho no Tab5; não há medição de FPS desta versão.
