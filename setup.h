/*
 * setup.h  —  VERSIÓN CORREGIDA
 */

#ifndef SETUP_H_
#define SETUP_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* ── UART ────────────────────────────────────────────────────────────── */
#define BAUD_RATE  57600
#define MYUBRR     ((F_CPU / 16 / BAUD_RATE) - 1)

/* ── I2C ─────────────────────────────────────────────────────────────── */
#define SCL_CLOCK  100000UL

/* ── Modo de simulación ──────────────────────────────────────────────── */
/* Descomentar para Proteus, comentar para hardware real                  */
#define PROTEUS_SIM

/* ── MPU-6050 ────────────────────────────────────────────────────────── */
#define LEN               14
#define MPU6050_ADDR      0x68

#define MPU6050_ADDR_W    ((MPU6050_ADDR << 1) | 0)
#define MPU6050_ADDR_R    ((MPU6050_ADDR << 1) | 1)

#define REG_PWR_MGMT_1    0x6B
#define REG_SMPLRT_DIV    0x19
#define REG_CONFIG        0x1A

//giroscopio
#define REG_GYRO_CONFIG   0x1B
#define REG_GYRO_XOUT	  0x43
#define REG_GYRO_YOUT	  0x45
#define REG_GYRO_ZOUT	  0x47
//Acelerometro
#define REG_ACCEL_CONFIG  0x1C
#define REG_ACCEL_XOUT_H  0x3B
#define REG_ACCEL_YOUT_H  0x3D
#define REG_ACCEL_ZOUT_H  0x3F


/* ── Filtro complementario ───────────────────────────────────────────── */
#define ALPHA   0.98f   /* FIX #7: float, no double                       */
#define DT      0.005f  /* FIX #2: periodo de muestreo 5 ms = 200 Hz      */

/* ── Servo PWM (ICR=40000, prescaler=8, F_CPU=16MHz → tick=0.5µs) ───── */
#define SERVO_MIN  2000   /* 1.0 ms */
#define SERVO_CTR  3000   /* 1.5 ms */
#define SERVO_MAX  4000   /* 2.0 ms */
#define ANGULO_MAX 45.0f

/* ── Variables globales (definidas en impl.c) ────────────────────────── */
extern volatile int16_t  ax, ay, az, t, gx, gy, gz;
extern float             Tp;
extern volatile char     contador;
extern volatile uint8_t  BUFFER[LEN];  

/* ── Prototipos ──────────────────────────────────────────────────────── */

/* UART */
void    USART_Init(uint16_t baud);
void    USART_Transmit(uint8_t data);
void    envia_mess(const char *s);

/* I2C */
void    i2c_init(void);
uint8_t i2c_start(uint8_t address_rw);
void    i2c_stop(void);
uint8_t i2c_write(uint8_t dato);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);

/* PWM */
void    PWM_init(void);

/* MPU-6050 */
void    mpu6050_write_reg(uint8_t reg, uint8_t val);
//void    mpu6050_read_regs(uint8_t start_reg);
int16_t mpu6050_read_16bit(uint8_t reg);
void    MPU6050_INICIALIZA(void);

/* Utils */
int16_t UNION(uint8_t hi, uint8_t lo);
void    SEND_INT32(int16_t valor);
void    envia_datos_raw(void);
void    calcular_angulos(float *pitch, float *roll); /* FIX #3: firma correcta */

/* Servo */
uint16_t angulo_a_ocr(float angulo);

/* ADC */
void ADC_init();
unsigned int ADC_read(unsigned char adc_input);
void send (uint8_t ch);

#endif /* SETUP_H_ */