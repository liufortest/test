#include "mxd2660_gpio.h"

#define IIC_SCL_PIN GPIO_21
#define IIC_SDA_PIN GPIO_22
#define IIC_SCL_OUTHIGH gpio_init_output(IIC_SCL_PIN, GPIO_PULL_UP, 1)
#define IIC_SCL_OUTLOW gpio_init_output(IIC_SCL_PIN, GPIO_PULL_UP, 0)
#define IIC_SDA_OUTHIGH gpio_init_output(IIC_SDA_PIN, GPIO_PULL_UP, 1)
#define IIC_SDA_OUTLOW gpio_init_output(IIC_SDA_PIN, GPIO_PULL_UP, 0)
#define WRITE 0x00
#define READ  0x01



extern void dly_us(uint32 microseconds);

void i2c_pin_init(void)
{
	IIC_SCL_OUTHIGH;
	IIC_SDA_OUTHIGH;
}

void i2c_start(void)
{
	IIC_SDA_OUTHIGH;  //将PIN脚设置为GPIO模式，并设置为输出，然后输出高
	IIC_SCL_OUTHIGH;
	dly_us(10);
	IIC_SDA_OUTLOW;
	dly_us(10);
	IIC_SCL_OUTLOW;
	dly_us(10);

}

void i2c_stop(void)
{
	IIC_SCL_OUTLOW;
	dly_us(10);
	IIC_SDA_OUTLOW;
	dly_us(10);
	IIC_SCL_OUTHIGH;
	dly_us(10);
	IIC_SDA_OUTHIGH;
}

void i2c_one_clk(void)
{
	dly_us(5);
	IIC_SCL_OUTHIGH;
	dly_us(10);
	IIC_SCL_OUTLOW;
	dly_us(5);
}

void i2c_restart(void)
{
	dly_us(20);
	IIC_SDA_OUTHIGH;
	dly_us(10);		//10
	IIC_SCL_OUTHIGH;
	dly_us(20);
	IIC_SDA_OUTLOW;
	dly_us(10);		//10
	IIC_SCL_OUTLOW;
	dly_us(10);	
}

uint8 i2c_read_byte_ack(void)
{
	int8 i;
	uint8 data=0;
	gpio_init_input(IIC_SDA_PIN,GPIO_PULL_UP);
	for (i=7; i>=0; i--) 
	{
		if (gpio_read_input_bit(IIC_SDA_PIN))
		{
			data |= (0x01<<i);
		}
		i2c_one_clk();
	}		
	IIC_SDA_OUTLOW;
	i2c_one_clk();
	return data;
}

uint8 i2c_read_byte_nack(void)
{
	int8 i;
	uint8 data=0;
	gpio_init_input(IIC_SDA_PIN,GPIO_PULL_UP);
	for (i=7; i>=0;i--) 
	{
		if (gpio_read_input_bit(IIC_SDA_PIN))
		{
			data |= (0x01<<i);
		}
		i2c_one_clk();
	}		
	IIC_SDA_OUTHIGH;
	i2c_one_clk();
	return data;
}	

void i2c_send_data(uint8 data)
{
	int8 i;
	for(i=7;i>=0;i--)
	{
		if((data>>i)&0x01)
		{
			IIC_SDA_OUTHIGH;
		}
		else
		{
			IIC_SDA_OUTLOW;
		}
		i2c_one_clk();
	}
}

uint8 i2c_ChkAck(void)
{
	gpio_init_input(IIC_SDA_PIN,GPIO_PULL_UP);
	dly_us(5);		//
	IIC_SCL_OUTHIGH;
	dly_us(5);		//

	if(gpio_read_input_bit(IIC_SDA_PIN))		//Non-ack
	{
		dly_us(5);	//
		IIC_SCL_OUTLOW;
		dly_us(5);	//
		IIC_SDA_OUTLOW;
		
		return 0;
	}
	else					//Ack
	{
		dly_us(5);	//
		IIC_SCL_OUTLOW;
		dly_us(5);	//
		IIC_SDA_OUTLOW;

		return 1;
	}
}

void i2c_readbyte(uint8 i2c_addr,uint8 regaddr,uint8 *data)
{
	i2c_start();

	i2c_send_data((i2c_addr<<1)|WRITE); //send i2c addr&write
	if(i2c_ChkAck()==0)
	{
		APP_DBG(("IIC ADDR ACK FAIL in readbyte\r\n"));
		i2c_stop();
		return;
	}
	i2c_send_data(regaddr); //send i2c register addr&write
	if(i2c_ChkAck()==0)
	{
		APP_DBG(("IIC register ADDR ACK FAIL in readbyte\r\n"));
		i2c_stop();
		return;
	}
	i2c_restart();
	
	i2c_send_data((i2c_addr<<1)|READ); //send i2c addr&read
	if(i2c_ChkAck()==0)
	{
		APP_DBG(("IIC ADDR ACK FAIL in readbyte\r\n"));
		i2c_stop();
		return;
	}
	
	*data = i2c_read_byte_nack();
	i2c_stop();
}

void i2c_con_readbytes(uint8 i2c_addr,uint8 regaddr,uint8 *data,uint8 len)
{
	uint8 i,*data_ptr;
	data_ptr=data;
	i2c_start();

	i2c_send_data((i2c_addr<<1)|WRITE); //send i2c addr&write
	if(i2c_ChkAck()==0)
	{
		APP_DBG(("IIC ADDR ACK FAIL in readbyte\r\n"));
		i2c_stop();
		return;
	}
	i2c_send_data(regaddr); //send i2c register addr&write
	if(i2c_ChkAck()==0)
	{
		APP_DBG(("IIC register ADDR ACK FAIL in readbyte\r\n"));
		i2c_stop();
		return;
	}
	i2c_restart();
	
	i2c_send_data((i2c_addr<<1)|READ); //send i2c addr&read
	if(i2c_ChkAck()==0)
	{
		APP_DBG(("IIC ADDR ACK FAIL in readbyte\r\n"));
		i2c_stop();
		return;
	}
	for(i=len;i>1;i--)
	{
		*data_ptr = i2c_read_byte_ack();
		data_ptr++;
	}
	*data_ptr = i2c_read_byte_nack();
	i2c_stop();
}

void i2c_writebyte(uint8 i2c_addr,uint8 regaddr,uint8 data)
{
	i2c_start();

	i2c_send_data((i2c_addr<<1)|WRITE); //send i2c addr&write
	if(i2c_ChkAck()==0)
	{
		APP_DBG(("IIC ADDR ACK FAIL in readbyte\r\n"));
		i2c_stop();
		return;
	}
	i2c_send_data(regaddr); //send i2c register addr
	if(i2c_ChkAck()==0)
	{
		APP_DBG(("IIC register ADDR ACK FAIL in readbyte\r\n"));
		i2c_stop();
		return;
	}
	
	i2c_send_data(data); //send data
	if(i2c_ChkAck()==0)
	{
		APP_DBG(("IIC ADDR ACK FAIL in readbyte\r\n"));
		i2c_stop();
		return;
	}
	
	i2c_stop();

}
