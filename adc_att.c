#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define led_red   13
#define led_blue  12
#define led_green 11
#define botao_A   5
#define botao_joy 22  //botão do joystick
#define joy_X     27  //eixo X do joystick
#define joy_Y     26  //eixo Y do joystick
#define I2C_SDA   14
#define I2C_SCL   15
#define I2C_PORT  i2c1 //Porta I2C usada
#define endereco  0x3C //Endereço do display OLED SSD1306
#define WIDTH     128  //Largura do display
#define HEIGHT    64   //altura do display

static bool green_mode = false; // Define se o LED verde está ativado
static bool pwm_enabled = true; //Define se os LEDs PWM estão ativos
static bool border_mode = false; //Define se a borda do display está ativa

// Posição inicial do cursor no meio da tela 
int cursor_X = WIDTH / 2;      
int cursor_Y = HEIGHT / 2;

// FUNÇÃO PARA A POSIÇÃO DO CURSOR NO DISPLAY
void update_cursor(ssd1306_t *ssd, int joystick_X, int joystick_Y) {
    static int last_cursor_X = WIDTH / 2;
    static int last_cursor_Y = HEIGHT / 2;

    //Converte o valor do joystick para as coordenadas da tela
    cursor_X = (joystick_X * (WIDTH - 8)) / 4095;
    cursor_Y = (joystick_Y * (HEIGHT - 8)) / 4095;

    // Garantir que o cursor não ultrapasse os limites da tela
    if (cursor_X < 0) cursor_X = 0;
    if (cursor_X >= WIDTH - 8) cursor_X = WIDTH - 8;
    if (cursor_Y < 0) cursor_Y = 0;
    if (cursor_Y >= HEIGHT - 8) cursor_Y = HEIGHT - 8;

    // Limpar tela antes de desenhar (evita rastros)
    ssd1306_fill(ssd, false);

    // Desenhar quadrado 8x8 para o novo cursor
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            ssd1306_pixel(ssd, cursor_X + i, cursor_Y + j, true);
        }
    }

    // Desenhar borda 
    ssd1306_rect(ssd, 0, 0, WIDTH, HEIGHT, true, false);

    // Atualizar display
    ssd1306_send_data(ssd);
}

//FUNÇÃO PARA INICIAR UM PINO COMO PWM
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}

//Variáveis para tratar interrupção
static volatile uint32_t last_time = 0; //para debounce
static void gpio_irq_handler(uint gpio, uint32_t events);

//FUNÇÃO PARA INTERRUPÇÃO
void gpio_irq_handler(uint gpio, uint32_t events) 
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if ((current_time - last_time) < 200000) return; //tratamento para bounce, evitando multiplos acionamento dos botões 
    last_time = current_time;

    if (gpio == botao_joy) //Altera o estado do led verde quando for pressionado
    {
        green_mode = !green_mode;
    }
    
    if (gpio == botao_A) //liga ou desliga os LEDs 
    {
        pwm_enabled = !pwm_enabled;
    }
}

//FUNÇÃO PRINCIPAL
int main() 
{   
    //Incializa comunicação I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    //Inicializa o display OLED
    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    
    //Inicializa os botões
    gpio_init(botao_A);
    gpio_set_dir(botao_A, GPIO_IN);
    gpio_pull_up(botao_A);

    gpio_init(botao_joy);
    gpio_set_dir(botao_joy, GPIO_IN);
    gpio_pull_up(botao_joy);

    //Inicializa o ADC para ler os valor analógicos do joystick
    adc_init();
    adc_gpio_init(joy_X);
    adc_gpio_init(joy_Y);

    //Configuração do PWM para os leds RGB
    uint pwm_wrap = 4095;
    uint pwm_red = pwm_init_gpio(led_red, pwm_wrap);
    uint pwm_blue = pwm_init_gpio(led_blue, pwm_wrap);
    uint pwm_green = pwm_init_gpio(led_green, pwm_wrap);

    //Tratamento para as interrupções
    gpio_set_irq_enabled_with_callback(botao_A, GPIO_IRQ_EDGE_RISE, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_joy, GPIO_IRQ_EDGE_RISE, true, &gpio_irq_handler);

    while (true) 
    {
        // Leitura dos valores do joystick
        adc_select_input(1);
        uint16_t joy_X_value = adc_read();
        adc_select_input(0);
        uint16_t joy_Y_value = adc_read();

        //Controle dos LEDs como PWM
        if (pwm_enabled) {
            if (!green_mode) //quando o botão do joystick não for pressionado
            {
                uint16_t pwm_red_value = abs(joy_X_value - 2048) * 2;
                uint16_t pwm_blue_value = abs(joy_Y_value - 2048) * 2;

                // Controla os LEDs vermelho e azul conforme o movimento do joystick
                pwm_set_gpio_level(led_red, (abs(joy_X_value - 2048) < 100) ? 0 : pwm_red_value);
                pwm_set_gpio_level(led_blue, (abs(joy_Y_value - 2048) < 100) ? 0 : pwm_blue_value);
                pwm_set_gpio_level(led_green, 0);
            } 
            else {   //quando o botão do joystick for pressionado
                pwm_set_gpio_level(led_red, 0);
                pwm_set_gpio_level(led_blue, 0);
                pwm_set_gpio_level(led_green, pwm_wrap);
            }
        } 
        else { //Desativa todos os leds quando o botão A for pressionado 
            pwm_set_gpio_level(led_red, 0);
            pwm_set_gpio_level(led_blue, 0);
            pwm_set_gpio_level(led_green, 0);
        }
        
        // Atualizar posição do cursor com os valores do joystick
        update_cursor(&ssd, joy_X_value, joy_Y_value);
        sleep_ms(50);
    }
    return 0;
}