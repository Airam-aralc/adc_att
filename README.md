![68747470733a2f2f736f667465782e62722f77702d636f6e74656e742f75706c6f6164732f323032342f30392f456d6261726361546563685f6c6f676f5f417a756c2d31303330783432382e706e67](https://github.com/user-attachments/assets/f7b310b2-600b-4fce-b0bc-f0a879db4533)

# Atividade sobre ADC_Conversor analógico-digital

Este projeto tem como objetivo consolidar o uso do conversor analógico-digital, presentes nos pinos GPIO 26, GIO 27 e GPIO 28 no microcontrolador RP2040, explorando as funcionalidades da placa BitDogLab, com o periférico Joystick. A implementação inclui o controle de LEDs RGB com PWM e demonstração gráfica de um cursor no display OLED ssd130, além do uso de botões com debouncing via software para o acionamento dos LEDs RGB.

Vídeo demonstrativo da atividade: https://www.dropbox.com/scl/fi/txqkg6rydm7d67jo3tzg7/Atividade-ADC.mp4?rlkey=s51pndag3lzk6oggksbxsmurp&st=cp4saw7k&dl=0

Código executável: adc_att.c

🛠 Funcionalidades

✅ Controle dos LED RGB, com os pinos conectados às GPIOs (11, 12 e 13), via PWM.

✅ Joystick conectado aos GPIOs 26 e 27, responsável em controlar a intensidade luminosa dos leds azul e vermelho.

✅ Display SSD1306 conectado via I2C (GPIO 14 e GPIO15), para mostrar um cursor de 8x8 pixels a medida da posição do joystick.

✅ Interrupções (IRQ) para detecção eficiente dos botões.

✅ Debouncing via software para evitar leituras indesejadas, no botão do Joystick conectado à GPIO 22 e no botão A conectado à GPIO 5.

O projeto foi testado e validado na placa BitDogLab, garantindo o funcionamento conforme os requisitos.
