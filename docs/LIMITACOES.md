# Limitações e aceitação física

Código e build disponíveis; a aceitação final de `REQUISITOS.md` exige testes
no Tab5. Nenhum periférico foi validado fisicamente nesta etapa.

| Item | Estado |
|---|---|
| ESP32-P4/ESP-IDF 5.4.4 | Build real passou; BINs OTA/USB gerados |
| Regras, entrada, journal, menus | Seis executáveis nativos com ASan/UBSan passaram |
| LCD/touch | BSP autodetecta ILI9881+GT911, ST7123/ST7121; confirmar etiqueta/log no aparelho |
| Teclado A164 | Normal pesquisado; estado simultâneo testado em software; polling físico pendente |
| Áudio | ES8388/I²S e síntese implementados; alto-falante/volume/ruído pendentes |
| Launcher | BIN válido abaixo do teto conservador; instalação/boot pendentes |
| FPS/tearing | Contador de render LVGL implementado; desempenho físico sem medição |
| microSD | Backup FAT/SDMMC compilado; cartão e escrita física pendentes |
| Persistência sem cartão | Duas gerações NVS; capacidade depende da tabela instalada |

Checklist no aparelho:

- Instalar app OTA, iniciar a frio e verificar landscape/variante LCD.
- Navegar pelo teclado; manter aceleração+direção+turbo e soltar cada tecla.
- Jogar cada modo/dificuldade; conferir kicks, hold, spins, ghost, lock delay.
- Conferir alimentos/efeitos, wrap, obstáculos e colisões do Snake; timer 2 min.
- Conferir cinco carros, tráfego, colisões/imunidade, turbo e meta 3 km/60 s.
- Salvar os três jogos, reiniciar e continuar com estado/RNG/efeitos intactos.
- Registrar nomes/TOP10, alterar preferências, reiniciar e verificar dados.
- Testar som/mudo e backup FAT com/sem cartão.
- Observar FPS por 60 s em cada jogo. Contador mede renders LVGL concluídos,
  não scanouts do painel. Block Drop evita redraws quando nada visível muda;
  FPS baixo em trechos estáticos não representa a taxa da simulação.

Limites explícitos:

- NVS de 24 KiB pode esgotar com várias categorias; standalone prevê 128 KiB.
  Falhas aparecem sem apagar dados ou mudar partições. Backup SD não amplia NVS.
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
