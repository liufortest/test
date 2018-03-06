#define	SPI_CS_HIGH()    Cy_GPIO_Write(NFC_CS_PORT, NFC_CS_PIN, 1)
#define	SPI_CS_LOW()     Cy_GPIO_Write(NFC_CS_PORT, NFC_CS_PIN, 0)

#define	SPI_MOSI_HIGH()    Cy_GPIO_Write(NFC_MOSI_PORT, NFC_MOSI_PIN, 1)
#define	SPI_MOSI_LOW()     Cy_GPIO_Write(NFC_MOSI_PORT, NFC_MOSI_PIN, 0)    

#define	SPI_CLK_HIGH()    Cy_GPIO_Write(NFC_CLK_PORT, NFC_CLK_PIN, 1)
#define	SPI_CLK_LOW()     Cy_GPIO_Write(NFC_CLK_PORT, NFC_CLK_PIN, 0)

#define SPI_MISO_READ()    Cy_GPIO_Read(NFC_MISO_PORT,NFC_MISO_PIN)

//=========================NFC SPI读函数（IO模拟SPI时序）==================
uint8_t NFC_SPI_Read_Byte(void)
{
    uint8_t i;
    uint8_t dat=0x00;
    //SPI1_CS1_LOW();
    //Delay_ms(1);
    for(i=0;i<8;i++)
    {
        dat=dat<<1;
        SPI_CLK_HIGH();
        asm("nop");asm("nop");
        if(SPI_MISO_READ()) //读取最高位，保存至最末尾，通过左移位完成整个字节
        {
            dat|=0x01;
        }
        SPI_CLK_LOW();
        asm("nop");asm("nop");
     }
    //SPI1_CS1_HIGH();
    return dat;
}

//=========================NFC SPI读写函数（IO模拟SPI时序）==================
void NFC_SPI_Write_Byte(uint8_t send)
{
    uint8_t i;
    //uint8_t dat=0x00;
    //SPI1_CS1_LOW();
    //Delay_ms(1);
    //dat=send;
    for(i=0;i<8;i++)
    {
        if(send&0x80) //总是发送最高位
        {
            SPI_MOSI_HIGH();
        }
        else
        {
            SPI_MOSI_LOW();
        }
        asm("nop");asm("nop");
        SPI_CLK_HIGH();
        asm("nop");asm("nop");
        SPI_CLK_LOW();
        send<<=1;
    }
    //SPI1_CS1_HIGH();
}
/**
 ****************************************************************
 * @brief write_reg()
 *
 * 写芯片的寄存器
 *
 * @param:  addr 寄存器地址
 ****************************************************************
 */
void write_reg(u8 addr, u8 val)
{
	u8 c;
	//最低位空闲，有效数据域为bit1~bit6
	addr <<= 1;
	//地址最高位为1代表读，为0代表写；
	c = addr & ~(READ_REG_CTRL);
	SPI_CS_LOW();
	NFC_SPI_Write_Byte(c);
	NFC_SPI_Write_Byte(val);
    SPI_CS_HIGH();
	
}

/**
 ****************************************************************
 * @brief read_reg()
 *
 * 读芯片的寄存器
 *
 * @param: addr 寄存器地址
 * @return: c 寄存器的值
 ****************************************************************
 */
u8 read_reg(u8 addr)
{
	u8 c;
	//最低位空闲，有效数据域为bit1~bit6
	addr <<= 1;
	//地址最高位为1代表读，为0代表写；
	c = addr | READ_REG_CTRL;
	SPI_CS_LOW();
	NFC_SPI_Write_Byte(c);
	c = NFC_SPI_Read_Byte();
	SPI_CS_HIGH();
	return c;
}