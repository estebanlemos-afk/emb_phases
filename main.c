/*
 * main.c  —  VERSIÓN CORREGIDA
 */

#include "setup.h"

int main(void)
{
    i2c_init();
	ADC_init();
    USART_Init(MYUBRR);

    DDRB = 0xFF;    /* salida: debug contador ISR                         */
    DDRC = 0xFF;    /* FIX #5: salida — ISR escribe PORTC                 */

    envia_mess("INICIANDO...\r\n");
    MPU6050_INICIALIZA();
    PWM_init();     /* FIX #4: inicializar servos                         */

#ifdef PROTEUS_SIM
    envia_mess("MODO: PROTEUS\r\n");
#else
    envia_mess("MODO: HARDWARE\r\n");
#endif

    envia_mess("LECTURAS:\r\n");
    sei();

    float   angulo_pitch = 0.0f;
    float   angulo_roll  = 0.0f;
    char    buf[50];
	
	while (1) {
		/* 1. Leer cada registro de 16 bits por separado */
		ax = mpu6050_read_16bit(REG_ACCEL_XOUT_H);
		ay = mpu6050_read_16bit(REG_ACCEL_YOUT_H);
		az = mpu6050_read_16bit(REG_ACCEL_ZOUT_H);
		//t  = mpu6050_read_16bit(REG_TEMP_OUT);
		gx = mpu6050_read_16bit(REG_GYRO_XOUT);
		gy = mpu6050_read_16bit(REG_GYRO_YOUT);
		gz = mpu6050_read_16bit(REG_GYRO_ZOUT);

		/* 3. Temperatura */
		//Tp = (t / 340.0f) + 36.53f;

		/* 4. Ángulos con filtro complementario */
		calcular_angulos(&angulo_pitch, &angulo_roll);

		/* 6. Telemetría */
		envia_datos_raw();
		
		// Búferes para procesar los floats de Pitch y Roll
		char pitch_str[10];
		char roll_str[10];
		
		dtostrf(angulo_pitch, 5, 2, pitch_str);
		dtostrf(angulo_roll, 5, 2, roll_str);
		
		// Imprimimos usando %s en lugar de %f
		sprintf(buf, "PITCH:%s | ROLL:%s\r\n", pitch_str, roll_str);
		envia_mess(buf);

		/* Muestreo ajustado a 5ms para que coincida con DT=0.005 */
		_delay_ms(5);
		
		envia_mess("X=");
		send(0);
		envia_mess("Y=");
		send(1);
		envia_mess("\r\n");
		_delay_ms(250);
	}
}