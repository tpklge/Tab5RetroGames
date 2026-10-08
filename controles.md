# Controles

Teclado oficial: I²C 0x6D, SDA0/SCL1, INT50 sem ISR. Polling a cada 8 ms em
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
