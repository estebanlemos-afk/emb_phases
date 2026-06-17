#include "setup.h"

/* ── Variables globales ──────────────────────────────────────────────── */
volatile int16_t ax, ay, az, t, gx, gy, gz;
float            Tp;
volatile char    contador = 0;
volatile uint8_t BUFFER[LEN];   /* FIX #1: uint8_t — cada byte I2C es 8 bits */

/* ════════════════════════════════════════════════════════════════════════
   UART
   ════════════════════════════════════════════════════════════════════════ */
void USART_Init(uint16_t baud) {
    UBRR0H = (unsigned char)(baud >> 8);
    UBRR0L = (unsigned char)baud;
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_Transmit(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void envia_mess(const char *s) {
    while (*s) {
        USART_Transmit(*s++);
    }
}

/* ISR recepción UART — debug visual en PORTB/PORTC */
ISR(USART0_RX_vect) {
    unsigned char data = UDR0;
    PORTB = contador;
    PORTC = data;   /* DDRC debe estar en 0xFF — ver main.c */
    contador++;
}

/* ════════════════════════════════════════════════════════════════════════
   I2C 
   ════════════════════════════════════════════════════════════════════════ */
void i2c_init(void) {
    TWSR = 0;
    TWBR = (uint8_t)(((F_CPU / SCL_CLOCK) - 16) / 2);
    TWCR = (1 << TWEN);
}

uint8_t i2c_start(uint8_t address_rw) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    TWDR = address_rw;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return (TWSR & 0xF8);
}

void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    while (TWCR & (1 << TWSTO));
}

uint8_t i2c_write(uint8_t dato) {
    TWDR = dato;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return (TWSR & 0xF8);
}

uint8_t i2c_read_ack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t i2c_read_nack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}
/* ════════════════════════════════════════════════════════════════════════
   PWM — Timer1 (servos) + Timer3 (timón)
   ════════════════════════════════════════════════════════════════════════ */
void PWM_init(void) {
    DDRB |= (1 << PB5) | (1 << PB6) | (1 << PB7);
    TCCR1A = (1 << COM1A1) | (1 << COM1A0)
           | (1 << COM1B1) | (1 << COM1B0)
           | (1 << COM1C1) | (1 << COM1C0)
           | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    ICR1   = 40000;

    DDRE |= (1 << PE3);
    TCCR3A = (1 << COM3A1) | (1 << COM3A0) | (1 << WGM31);
    TCCR3B = (1 << WGM33)  | (1 << WGM32)  | (1 << CS31);
    ICR3   = 40000;

    OCR1A = SERVO_CTR;
    OCR1B = SERVO_CTR;
    OCR1C = SERVO_CTR;
    OCR3A = SERVO_CTR;
}

/* ════════════════════════════════════════════════════════════════════════
   MPU-6050
   ════════════════════════════════════════════════════════════════════════ */
void mpu6050_write_reg(uint8_t reg, uint8_t val) {
    i2c_start(MPU6050_ADDR_W);
    i2c_write(reg);
    i2c_write(val);
    i2c_stop();
}

int16_t mpu6050_read_16bit(uint8_t reg){
	i2c_start(MPU6050_ADDR_W);
	i2c_write(reg);
	i2c_start(MPU6050_ADDR_R);
	uint8_t hi = i2c_read_ack();   // Lee byte alto
	uint8_t lo = i2c_read_nack();  // Lee byte bajo y finaliza lectura
	i2c_stop();
	return (int16_t)((hi << 8) | lo);
}

void MPU6050_INICIALIZA(void) {
    _delay_ms(100);
    mpu6050_write_reg(REG_PWR_MGMT_1,  0x00);
    _delay_ms(10);
    mpu6050_write_reg(REG_CONFIG,       0x03);  /* DLPF 44 Hz        */
    mpu6050_write_reg(REG_SMPLRT_DIV,   0x04);  /* 200 Hz            */
    mpu6050_write_reg(REG_GYRO_CONFIG,  0x00);  /* ±250 °/s          */
    mpu6050_write_reg(REG_ACCEL_CONFIG, 0x10);  /* ±8 g → 4096 LSB/g*/
}

/* ════════════════════════════════════════════════════════════════════════
   UTILS
   ════════════════════════════════════════════════════════════════════════ */
int16_t UNION(uint8_t hi, uint8_t lo) {
    return (int16_t)((hi << 8) | lo);
}

void SEND_INT32(int16_t valor) {
    char buf[8];
    sprintf(buf, "%d", valor);
    envia_mess(buf);
}

void envia_datos_raw(void) {
	envia_mess("AX:"); SEND_INT32(ax);
	envia_mess(" AY:"); SEND_INT32(ay);
	envia_mess(" AZ:"); SEND_INT32(az);
	envia_mess("\r\nGX:"); SEND_INT32(gx);
	envia_mess(" GY:"); SEND_INT32(gy);
	envia_mess(" GZ:"); SEND_INT32(gz);
	envia_mess("\r\n");
}

/* ════════════════════════════════════════════════════════════════════════
   CÁLCULO DE ÁNGULOS — compilación condicional Proteus / hardware real
   FIX #3: firma corregida (2 parámetros)
   FIX #6: DT=0.005 coincide con _delay_ms(5) en main
   FIX #7: todas las constantes son float (sufijo f)
   ════════════════════════════════════════════════════════════════════════ */
void calcular_angulos(float *pitch, float *roll) {

    float ax_g = ax / 4096.0f;   /* ±8g → 4096 LSB/g */
    float ay_g = ay / 4096.0f;
    float az_g = az / 4096.0f;

    float pitch_acc, roll_acc;

#ifdef PROTEUS_SIM
    /*
     * Proteus orienta el sensor con gravedad en el eje -X por defecto.
     * Se reordenan los ejes para que pitch=0/roll=0 cuando el sensor
     * aparece "horizontal" en la simulación.
     *
     * Horizontal Proteus:  AX=-4096, AY=0, AZ=0
     * → pitch_acc = atan2(-0, sqrt(0+1)) = 0°  ✓
     * → roll_acc  = atan2( 0, 1)         = 0°  ✓
     */
    pitch_acc = atan2f(-az_g, sqrtf(ay_g*ay_g + ax_g*ax_g)) * (180.0f / M_PI);
    roll_acc  = atan2f( ay_g, -ax_g)                         * (180.0f / M_PI);
#else
    /*
     * Hardware real: gravedad en eje +Z cuando el sensor está horizontal.
     * Horizontal real:  AX=0, AY=0, AZ=+4096
     * → pitch_acc = atan2(0, sqrt(0+1)) = 0°  ✓
     * → roll_acc  = atan2(0, 1)         = 0°  ✓
     */
    pitch_acc = atan2f( ax_g, sqrtf(ay_g*ay_g + az_g*az_g)) * (180.0f / M_PI);
    roll_acc  = atan2f( ay_g, az_g)                          * (180.0f / M_PI);
#endif

    /* Giroscopio: ±250 °/s → 131 LSB/°/s */
    float gx_ds = gx / 131.0f;
    float gy_ds = gy / 131.0f;

    /* Filtro complementario */
    *pitch = ALPHA * (*pitch + gx_ds * DT) + (1.0f - ALPHA) * pitch_acc;
    *roll  = ALPHA * (*roll  + gy_ds * DT) + (1.0f - ALPHA) * roll_acc;
}

/* ════════════════════════════════════════════════════════════════════════
   SERVO
   ════════════════════════════════════════════════════════════════════════ */
uint16_t angulo_a_ocr(float angulo) {
    float ratio = angulo / ANGULO_MAX;
    if (ratio >  1.0f) ratio =  1.0f;
    if (ratio < -1.0f) ratio = -1.0f;
    return (uint16_t)(SERVO_CTR + ratio * (float)(SERVO_MAX - SERVO_CTR));
}
/* ════════════════════════════════════════════════════════════════════════
   ADC
   ════════════════════════════════════════════════════════════════════════ */
void ADC_init(){
	DIDR0=0x00;
	DIDR2=0x00;
	ADMUX=0x40;
	ADCSRA=0x84;
	ADCSRB=0x00;
}

unsigned int ADC_read(unsigned char adc_input){
	ADMUX=(adc_input & 0x1F) | 0X40;
	if(adc_input& 0x20) ADCSRB|=(1<<MUX5);
	else ADCSRB&=~(1<<MUX5);
	_delay_us(1);
	ADCSRA|=(1<<ADSC);
	while ((ADCSRA & (1<<ADIF))==0);
	ADCSRA|=(1<<ADIF);
	return ADCW;
}

void send (uint8_t ch){
	char buffer[12];
	int val=0;
	val=ADC_read(ch);
	sprintf(buffer,"%d",val);
	envia_mess(buffer);
}