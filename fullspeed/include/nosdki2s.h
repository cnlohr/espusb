#ifndef _NOSDK_I2S_H
#define _NOSDK_I2S_H

#include "esp8266_rom.h"
#include "nosdk8266.h"

//Functions you'll call:
void InitI2S();

void SendI2S();

//From i2s_reg.h
extern volatile uint32_t * DR_REG_I2S_BASEL;
extern volatile uint32_t * DR_REG_SLC_BASEL;
#define DR_REG_I2S_BASE 0x60000e00
#define DR_REG_SLC_BASE 0x60000B00



#define I2STXFIFO  (DR_REG_I2S_BASE + 0x0000)
#define I2SRXFIFO  (DR_REG_I2S_BASE + 0x0004)
#define I2SCONF  (DR_REG_I2S_BASE + 0x0008)
#define I2SCONFL  (DR_REG_I2S_BASEL[0x0008/4])
#define I2S_BCK_DIV_NUM 0x0000003F
#define I2S_BCK_DIV_NUM_S 22
#define I2S_CLKM_DIV_NUM 0x0000003F
#define I2S_CLKM_DIV_NUM_S 16
#define I2S_BITS_MOD 0x0000000F
#define I2S_BITS_MOD_S 12
#define I2S_RECE_MSB_SHIFT (BIT(11))
#define I2S_TRANS_MSB_SHIFT (BIT(10))
#define I2S_I2S_RX_START (BIT(9))
#define I2S_I2S_TX_START (BIT(8))
#define I2S_MSB_RIGHT (BIT(7))
#define I2S_RIGHT_FIRST (BIT(6))
#define I2S_RECE_SLAVE_MOD (BIT(5))
#define I2S_TRANS_SLAVE_MOD (BIT(4))
#define I2S_I2S_RX_FIFO_RESET (BIT(3))
#define I2S_I2S_TX_FIFO_RESET (BIT(2))
#define I2S_I2S_RX_RESET (BIT(1))
#define I2S_I2S_TX_RESET (BIT(0))
#define I2S_I2S_RESET_MASK 0xf

#define I2SINT_RAW (DR_REG_I2S_BASE + 0x000c)
#define I2S_I2S_TX_REMPTY_INT_RAW (BIT(5))
#define I2S_I2S_TX_WFULL_INT_RAW (BIT(4))
#define I2S_I2S_RX_REMPTY_INT_RAW (BIT(3))
#define I2S_I2S_RX_WFULL_INT_RAW (BIT(2))
#define I2S_I2S_TX_PUT_DATA_INT_RAW (BIT(1))
#define I2S_I2S_RX_TAKE_DATA_INT_RAW (BIT(0))


#define I2SINT_ST (DR_REG_I2S_BASE + 0x0010)
#define I2S_I2S_TX_REMPTY_INT_ST (BIT(5))
#define I2S_I2S_TX_WFULL_INT_ST (BIT(4))
#define I2S_I2S_RX_REMPTY_INT_ST (BIT(3))
#define I2S_I2S_RX_WFULL_INT_ST (BIT(2))
#define I2S_I2S_TX_PUT_DATA_INT_ST (BIT(1))
#define I2S_I2S_RX_TAKE_DATA_INT_ST (BIT(0))

#define I2SINT_ENA (DR_REG_I2S_BASE + 0x0014)
#define I2SINT_ENAL (DR_REG_I2S_BASEL[0x0014/4])
#define I2S_I2S_TX_REMPTY_INT_ENA (BIT(5))
#define I2S_I2S_TX_WFULL_INT_ENA (BIT(4))
#define I2S_I2S_RX_REMPTY_INT_ENA (BIT(3))
#define I2S_I2S_RX_WFULL_INT_ENA (BIT(2))
#define I2S_I2S_TX_PUT_DATA_INT_ENA (BIT(1))
#define I2S_I2S_RX_TAKE_DATA_INT_ENA (BIT(0))

#define I2SINT_CLR (DR_REG_I2S_BASE + 0x0018)
#define I2SINT_CLRL (DR_REG_I2S_BASEL[0x0018/4])
#define I2S_I2S_TX_REMPTY_INT_CLR (BIT(5))
#define I2S_I2S_TX_WFULL_INT_CLR (BIT(4))
#define I2S_I2S_RX_REMPTY_INT_CLR (BIT(3))
#define I2S_I2S_RX_WFULL_INT_CLR (BIT(2))
#define I2S_I2S_PUT_DATA_INT_CLR (BIT(1))
#define I2S_I2S_TAKE_DATA_INT_CLR (BIT(0))

#define I2STIMING (DR_REG_I2S_BASE + 0x001c)
#define I2S_TRANS_BCK_IN_INV (BIT(22))
#define I2S_RECE_DSYNC_SW (BIT(21))
#define I2S_TRANS_DSYNC_SW (BIT(20))
#define I2S_RECE_BCK_OUT_DELAY 0x00000003
#define I2S_RECE_BCK_OUT_DELAY_S 18
#define I2S_RECE_WS_OUT_DELAY 0x00000003
#define I2S_RECE_WS_OUT_DELAY_S 16
#define I2S_TRANS_SD_OUT_DELAY 0x00000003
#define I2S_TRANS_SD_OUT_DELAY_S 14
#define I2S_TRANS_WS_OUT_DELAY 0x00000003
#define I2S_TRANS_WS_OUT_DELAY_S 12
#define I2S_TRANS_BCK_OUT_DELAY 0x00000003
#define I2S_TRANS_BCK_OUT_DELAY_S 10
#define I2S_RECE_SD_IN_DELAY 0x00000003
#define I2S_RECE_SD_IN_DELAY_S 8
#define I2S_RECE_WS_IN_DELAY 0x00000003
#define I2S_RECE_WS_IN_DELAY_S 6
#define I2S_RECE_BCK_IN_DELAY 0x00000003
#define I2S_RECE_BCK_IN_DELAY_S 4
#define I2S_TRANS_WS_IN_DELAY 0x00000003
#define I2S_TRANS_WS_IN_DELAY_S 2
#define I2S_TRANS_BCK_IN_DELAY 0x00000003
#define I2S_TRANS_BCK_IN_DELAY_S 0

#define I2S_FIFO_CONF (DR_REG_I2S_BASE + 0x0020)
#define I2S_FIFO_CONFL (DR_REG_I2S_BASEL[0x0020/4])
#define I2S_I2S_RX_FIFO_MOD 0x00000007
#define I2S_I2S_RX_FIFO_MOD_S 16
#define I2S_I2S_TX_FIFO_MOD 0x00000007
#define I2S_I2S_TX_FIFO_MOD_S 13
#define I2S_I2S_DSCR_EN (BIT(12))
#define I2S_I2S_TX_DATA_NUM 0x0000003F
#define I2S_I2S_TX_DATA_NUM_S 6
#define I2S_I2S_RX_DATA_NUM 0x0000003F
#define I2S_I2S_RX_DATA_NUM_S 0


#define I2SRXEOF_NUM (DR_REG_I2S_BASE + 0x0024)
#define I2S_I2S_RX_EOF_NUM 0xFFFFFFFF
#define I2S_I2S_RX_EOF_NUM_S 0

#define I2SCONF_SIGLE_DATA (DR_REG_I2S_BASE + 0x0028)
#define I2S_I2S_SIGLE_DATA 0xFFFFFFFF
#define I2S_I2S_SIGLE_DATA_S 0

#define I2SCONF_CHAN (DR_REG_I2S_BASE + 0x002c)
#define I2S_RX_CHAN_MOD 0x00000003
#define I2S_RX_CHAN_MOD_S 3
#define I2S_TX_CHAN_MOD 0x00000007
#define I2S_TX_CHAN_MOD_S 0

#ifndef i2c_bbpll
#define i2c_bbpll                                 0x67
#define i2c_bbpll_en_audio_clock_out            4
#define i2c_bbpll_en_audio_clock_out_msb        7
#define i2c_bbpll_en_audio_clock_out_lsb        7
#define i2c_bbpll_hostid                           4

#define i2c_writeReg_Mask(block, host_id, reg_add, Msb, Lsb, indata)  rom_i2c_writeReg_Mask(block, host_id, reg_add, Msb, Lsb, indata)
#define i2c_readReg_Mask(block, host_id, reg_add, Msb, Lsb)  rom_i2c_readReg_Mask(block, host_id, reg_add, Msb, Lsb)
#define i2c_writeReg_Mask_def(block, reg_add, indata) \
      i2c_writeReg_Mask(block, block##_hostid,  reg_add,  reg_add##_msb,  reg_add##_lsb,  indata)
#define i2c_readReg_Mask_def(block, reg_add) \
      i2c_readReg_Mask(block, block##_hostid,  reg_add,  reg_add##_msb,  reg_add##_lsb)
#endif
#ifndef ETS_SLC_INUM
#define ETS_SLC_INUM       1
#endif

struct sdio_queue
{
	uint32	blocksize:12;
	uint32	datalen:12;
	uint32	unused:5;
	uint32	sub_sof:1;
	uint32 	eof:1;
	uint32	owner:1;

	uint32	buf_ptr;
	uint32	next_link_ptr;
};

struct sdio_slave_status_element
{
	uint32 wr_busy:1;
	uint32 rd_empty :1;
	uint32 comm_cnt :3;
	uint32 intr_no :3;
	uint32 rx_length:16;
	uint32 res:8;
};

union sdio_slave_status
{
	struct sdio_slave_status_element elm_value;
	uint32 word_value;
};

#define RX_AVAILIBLE 2
#define TX_AVAILIBLE 1
#define INIT_STAGE	 0

#define SDIO_QUEUE_LEN 8
#define MOSI  0
#define MISO  1
#define SDIO_DATA_ERROR 6

#define SLC_INTEREST_EVENT (SLC_TX_EOF_INT_ENA | SLC_RX_EOF_INT_ENA | SLC_RX_UDF_INT_ENA | SLC_TX_DSCR_ERR_INT_ENA)
#define TRIG_TOHOST_INT()	SET_PERI_REG_MASK(SLC_INTVEC_TOHOST , BIT0);\
							CLEAR_PERI_REG_MASK(SLC_INTVEC_TOHOST , BIT0)



//From SLC Register...


#define SLC_CONF0                                (DR_REG_SLC_BASE + 0x0)
#define SLC_CONF0L                               (DR_REG_SLC_BASEL[0])
#ifndef ESP_MAC_5
#define SLC_MODE 0x00000003
#define SLC_MODE_S 12
#endif

#define SLC_DATA_BURST_EN (BIT(9))
#define SLC_DSCR_BURST_EN (BIT(8))
#define SLC_RX_NO_RESTART_CLR (BIT(7))
#define SLC_RX_AUTO_WRBACK (BIT(6))
#define SLC_RX_LOOP_TEST (BIT(5))
#define SLC_TX_LOOP_TEST (BIT(4))
#define SLC_AHBM_RST (BIT(3))
#define SLC_AHBM_FIFO_RST (BIT(2))
#define SLC_RXLINK_RST (BIT(1))
#define SLC_TXLINK_RST (BIT(0))

#define SLC_INT_RAW                              (DR_REG_SLC_BASE + 0x4)
#define SLC_INT_RAWL                              (DR_REG_SLC_BASEL[0x4/4])
#define SLC_TX_DSCR_EMPTY_INT_RAW (BIT(21))
#define SLC_RX_DSCR_ERR_INT_RAW (BIT(20))
#define SLC_TX_DSCR_ERR_INT_RAW (BIT(19))
#define SLC_TOHOST_INT_RAW (BIT(18))
#define SLC_RX_EOF_INT_RAW (BIT(17))
#define SLC_RX_DONE_INT_RAW (BIT(16))
#define SLC_TX_EOF_INT_RAW (BIT(15))
#define SLC_TX_DONE_INT_RAW (BIT(14))
#define SLC_TOKEN1_1TO0_INT_RAW (BIT(13))
#define SLC_TOKEN0_1TO0_INT_RAW (BIT(12))
#define SLC_TX_OVF_INT_RAW (BIT(11))
#define SLC_RX_UDF_INT_RAW (BIT(10))
#define SLC_TX_START_INT_RAW (BIT(9))
#define SLC_RX_START_INT_RAW (BIT(8))
#define SLC_FRHOST_BIT7_INT_RAW (BIT(7))
#define SLC_FRHOST_BIT6_INT_RAW (BIT(6))
#define SLC_FRHOST_BIT5_INT_RAW (BIT(5))
#define SLC_FRHOST_BIT4_INT_RAW (BIT(4))
#define SLC_FRHOST_BIT3_INT_RAW (BIT(3))
#define SLC_FRHOST_BIT2_INT_RAW (BIT(2))
#define SLC_FRHOST_BIT1_INT_RAW (BIT(1))
#define SLC_FRHOST_BIT0_INT_RAW (BIT(0))

#define SLC_INT_STATUS                               (DR_REG_SLC_BASE + 0x8)
#define SLC_INT_STATUSL                               (DR_REG_SLC_BASEL[0x8/4])
#define SLC_TX_DSCR_EMPTY_INT_ST (BIT(21))
#define SLC_RX_DSCR_ERR_INT_ST (BIT(20))
#define SLC_TX_DSCR_ERR_INT_ST (BIT(19))
#define SLC_TOHOST_INT_ST (BIT(18))
#define SLC_RX_EOF_INT_ST (BIT(17))
#define SLC_RX_DONE_INT_ST (BIT(16))
#define SLC_TX_EOF_INT_ST (BIT(15))
#define SLC_TX_DONE_INT_ST (BIT(14))
#define SLC_TOKEN1_1TO0_INT_ST (BIT(13))
#define SLC_TOKEN0_1TO0_INT_ST (BIT(12))
#define SLC_TX_OVF_INT_ST (BIT(11))
#define SLC_RX_UDF_INT_ST (BIT(10))
#define SLC_TX_START_INT_ST (BIT(9))
#define SLC_RX_START_INT_ST (BIT(8))
#define SLC_FRHOST_BIT7_INT_ST (BIT(7))
#define SLC_FRHOST_BIT6_INT_ST (BIT(6))
#define SLC_FRHOST_BIT5_INT_ST (BIT(5))
#define SLC_FRHOST_BIT4_INT_ST (BIT(4))
#define SLC_FRHOST_BIT3_INT_ST (BIT(3))
#define SLC_FRHOST_BIT2_INT_ST (BIT(2))
#define SLC_FRHOST_BIT1_INT_ST (BIT(1))
#define SLC_FRHOST_BIT0_INT_ST (BIT(0))

#define SLC_INT_ENA                              (DR_REG_SLC_BASE + 0xC)
#define SLC_INT_ENAL                              (DR_REG_SLC_BASEL[0xC/4])
#define SLC_TX_DSCR_EMPTY_INT_ENA (BIT(21))
#define SLC_RX_DSCR_ERR_INT_ENA (BIT(20))
#define SLC_TX_DSCR_ERR_INT_ENA (BIT(19))
#define SLC_TOHOST_INT_ENA (BIT(18))
#define SLC_RX_EOF_INT_ENA (BIT(17))
#define SLC_RX_DONE_INT_ENA (BIT(16))
#define SLC_TX_EOF_INT_ENA (BIT(15))
#define SLC_TX_DONE_INT_ENA (BIT(14))
#define SLC_TOKEN1_1TO0_INT_ENA (BIT(13))
#define SLC_TOKEN0_1TO0_INT_ENA (BIT(12))
#define SLC_TX_OVF_INT_ENA (BIT(11))
#define SLC_RX_UDF_INT_ENA (BIT(10))
#define SLC_TX_START_INT_ENA (BIT(9))
#define SLC_RX_START_INT_ENA (BIT(8))
#define SLC_FRHOST_BIT7_INT_ENA (BIT(7))
#define SLC_FRHOST_BIT6_INT_ENA (BIT(6))
#define SLC_FRHOST_BIT5_INT_ENA (BIT(5))
#define SLC_FRHOST_BIT4_INT_ENA (BIT(4))
#define SLC_FRHOST_BIT3_INT_ENA (BIT(3))
#define SLC_FRHOST_BIT2_INT_ENA (BIT(2))
#define SLC_FRHOST_BIT1_INT_ENA (BIT(1))
#define SLC_FRHOST_BIT0_INT_ENA (BIT(0))

#define SLC_FRHOST_BIT_INT_ENA_ALL  0xff

#define SLC_INT_CLR                              (DR_REG_SLC_BASE + 0x10)
#define SLC_INT_CLRL                             (DR_REG_SLC_BASEL[4])
#define SLC_TX_DSCR_EMPTY_INT_CLR (BIT(21))
#define SLC_RX_DSCR_ERR_INT_CLR (BIT(20))
#define SLC_TX_DSCR_ERR_INT_CLR (BIT(19))
#define SLC_TOHOST_INT_CLR (BIT(18))
#define SLC_RX_EOF_INT_CLR (BIT(17))
#define SLC_RX_DONE_INT_CLR (BIT(16))
#define SLC_TX_EOF_INT_CLR (BIT(15))
#define SLC_TX_DONE_INT_CLR (BIT(14))
#define SLC_TOKEN1_1TO0_INT_CLR (BIT(13))
#define SLC_TOKEN0_1TO0_INT_CLR (BIT(12))
#define SLC_TX_OVF_INT_CLR (BIT(11))
#define SLC_RX_UDF_INT_CLR (BIT(10))
#define SLC_TX_START_INT_CLR (BIT(9))
#define SLC_RX_START_INT_CLR (BIT(8))
#define SLC_FRHOST_BIT7_INT_CLR (BIT(7))
#define SLC_FRHOST_BIT6_INT_CLR (BIT(6))
#define SLC_FRHOST_BIT5_INT_CLR (BIT(5))
#define SLC_FRHOST_BIT4_INT_CLR (BIT(4))
#define SLC_FRHOST_BIT3_INT_CLR (BIT(3))
#define SLC_FRHOST_BIT2_INT_CLR (BIT(2))
#define SLC_FRHOST_BIT1_INT_CLR (BIT(1))
#define SLC_FRHOST_BIT0_INT_CLR (BIT(0))

#define SLC_RX_STATUS                            (DR_REG_SLC_BASE + 0x14)
#define SLC_RX_EMPTY (BIT(1))
#define SLC_RX_FULL (BIT(0))

#define SLC_RX_FIFO_PUSH                          (DR_REG_SLC_BASE + 0x18)
#define SLC_RXFIFO_PUSH (BIT(16))
#define SLC_RXFIFO_WDATA 0x000001FF
#define SLC_RXFIFO_WDATA_S 0

#define SLC_TX_STATUS                            (DR_REG_SLC_BASE + 0x1C)
#define SLC_TX_EMPTY (BIT(1))
#define SLC_TX_FULL (BIT(0))

#define SLC_TX_FIFO_POP                           (DR_REG_SLC_BASE + 0x20)
#define SLC_TXFIFO_POP (BIT(16))
#define SLC_TXFIFO_RDATA 0x000007FF
#define SLC_TXFIFO_RDATA_S 0

#define SLC_RX_LINK                              (DR_REG_SLC_BASE + 0x24)
#define SLC_RX_LINKL                              (DR_REG_SLC_BASEL[0x24/4])
#define SLC_RXLINK_PARK (BIT(31))
#define SLC_RXLINK_RESTART (BIT(30))
#define SLC_RXLINK_START  (BIT(29))
#define SLC_RXLINK_STOP  (BIT(28))
#define SLC_RXLINK_DESCADDR_MASK 0x000FFFFF
#define SLC_RXLINK_ADDR_S 0

#define SLC_TX_LINK                              (DR_REG_SLC_BASE + 0x28)
#define SLC_TXLINK_PARK (BIT(31))
#define SLC_TXLINK_RESTART (BIT(30))
#define SLC_TXLINK_START  (BIT(29))
#define SLC_TXLINK_STOP  (BIT(28))
#define SLC_TXLINK_DESCADDR_MASK 0x000FFFFF
#define SLC_TXLINK_ADDR_S 0

#define SLC_INTVEC_TOHOST                        (DR_REG_SLC_BASE + 0x2C)
#define SLC_TOHOST_INTVEC 0x000000FF
#define SLC_TOHOST_INTVEC_S 0

#define SLC_TOKEN0                               (DR_REG_SLC_BASE + 0x30)
#define SLC_TOKEN0_MASK 0x00000FFF
#define SLC_TOKEN0_S 16
#define SLC_TOKEN0_LOCAL_INC_MORE (BIT(14))
#define SLC_TOKEN0_LOCAL_INC (BIT(13))
#define SLC_TOKEN0_LOCAL_WR (BIT(12))
#define SLC_TOKEN0_LOCAL_WDATA_MASK 0x00000FFF
#define SLC_TOKEN0_LOCAL_WDATA_S 0

#define SLC_TOKEN1                               (DR_REG_SLC_BASE + 0x34)
#define SLC_TOKEN1_MASK 0x00000FFF
#define SLC_TOKEN1_S 16
#define SLC_TOKEN1_LOCAL_INC_MORE (BIT(14))
#define SLC_TOKEN1_LOCAL_INC (BIT(13))
#define SLC_TOKEN1_LOCAL_WR (BIT(12))
#define SLC_TOKEN1_LOCAL_WDATA 0x00000FFF
#define SLC_TOKEN1_LOCAL_WDATA_S 0

#define SLC_CONF1                                (DR_REG_SLC_BASE + 0x38)
#define SLC_STATE0                               (DR_REG_SLC_BASE + 0x3C)
#define SLC_STATE1                               (DR_REG_SLC_BASE + 0x40)

#define SLC_BRIDGE_CONF                          (DR_REG_SLC_BASE + 0x44)
#ifndef ESP_MAC_5
#define SLC_TX_PUSH_IDLE_NUM 0x0000FFFF
#define SLC_TX_PUSH_IDLE_NUM_S 16
#define SLC_TX_DUMMY_MODE (BIT(12))
#endif
#define SLC_FIFO_MAP_ENA 0x0000000F
#define SLC_FIFO_MAP_ENA_S 8
#define SLC_TXEOF_ENA  0x0000003F
#define SLC_TXEOF_ENA_S 0

#define SLC_RX_EOF_DES_ADDR                       (DR_REG_SLC_BASE + 0x48)
#define SLC_TX_EOF_DES_ADDR                       (DR_REG_SLC_BASE + 0x4C)
#define SLC_FROM_HOST_LAST_DESC                   SLC_TX_EOF_DES_ADDR
#define SLC_TO_HOST_LAST_DESC                     SLC_RX_EOF_DES_ADDR

#define SLC_RX_EOF_BFR_DES_ADDR                 (DR_REG_SLC_BASE + 0x50)
#define SLC_AHB_TEST                            (DR_REG_SLC_BASE + 0x54)
#define SLC_AHB_TESTADDR 0x00000003
#define SLC_AHB_TESTADDR_S 4
#define SLC_AHB_TESTMODE 0x00000007
#define SLC_AHB_TESTMODE_S 0

#define SLC_SDIO_ST                             (DR_REG_SLC_BASE + 0x58)
#define SLC_BUS_ST 0x00000007
#define SLC_BUS_ST_S 12
#define SLC_SDIO_WAKEUP (BIT(8))
#define SLC_FUNC_ST 0x0000000F
#define SLC_FUNC_ST_S 4
#define SLC_CMD_ST 0x00000007
#define SLC_CMD_ST_S 0

#define SLC_RX_DSCR_CONF                        (DR_REG_SLC_BASE + 0x5C)
#define SLC_RX_DSCR_CONFL                       (DR_REG_SLC_BASEL[0x5C/4])
#ifdef ESP_MAC_5
#define SLC_INFOR_NO_REPLACE (BIT(9))
#define SLC_TOKEN_NO_REPLACE (BIT(8))
#define SLC_POP_IDLE_CNT 0x000000FF
#else
#define SLC_RX_FILL_EN (BIT(20))
#define SLC_RX_EOF_MODE (BIT(19))
#define SLC_RX_FILL_MODE (BIT(18))
#define SLC_INFOR_NO_REPLACE (BIT(17))
#define SLC_TOKEN_NO_REPLACE (BIT(16))
#define SLC_POP_IDLE_CNT 0x0000FFFF
#endif
#define SLC_POP_IDLE_CNT_S 0

#define SLC_TXLINK_DSCR                         (DR_REG_SLC_BASE + 0x60)
#define SLC_TXLINK_DSCR_BF0                     (DR_REG_SLC_BASE + 0x64)
#define SLC_TXLINK_DSCR_BF1                     (DR_REG_SLC_BASE + 0x68)
#define SLC_RXLINK_DSCR                         (DR_REG_SLC_BASE + 0x6C)
#define SLC_RXLINK_DSCR_BF0                     (DR_REG_SLC_BASE + 0x70)
#define SLC_RXLINK_DSCR_BF1                     (DR_REG_SLC_BASE + 0x74)
#define SLC_DATE                                 (DR_REG_SLC_BASE + 0x78)
#define SLC_ID                                   (DR_REG_SLC_BASE + 0x7C)

#define SLC_HOST_CONF_W0                         (DR_REG_SLC_BASE + 0x80 + 0x14)
#define SLC_HOST_CONF_W1                         (DR_REG_SLC_BASE + 0x80 + 0x18)
#define SLC_HOST_CONF_W2                         (DR_REG_SLC_BASE + 0x80 + 0x20)
#define SLC_HOST_CONF_W3                         (DR_REG_SLC_BASE + 0x80 + 0x24)
#define SLC_HOST_CONF_W4                         (DR_REG_SLC_BASE + 0x80 + 0x28)

#define SLC_HOST_INTR_ST                         (DR_REG_SLC_BASE + 0x80 + 0x1c)
#define SLC_HOST_INTR_CLR                         (DR_REG_SLC_BASE + 0x80 + 0x30)
#define SLC_HOST_INTR_SOF_BIT                     (BIT(12))

#define SLC_HOST_INTR_ENA                         (DR_REG_SLC_BASE + 0x80 + 0x34)
#define SLC_RX_NEW_PACKET_INT_ENA 		(BIT23)
#define SLC_HOST_TOHOST_BIT0_INT_ENA		(BIT0)
#define SLC_HOST_CONF_W5                          (DR_REG_SLC_BASE + 0x80 + 0x3C)
#define SLC_HOST_INTR_RAW                         (DR_REG_SLC_BASE + 0x80 + 0x8)
#define SLC_HOST_INTR_ENA_BIT                      (BIT(23))
//[15:12]: 0x3ff9xxxx -- 0b01  from_host
//         0x3ffaxxxx -- 0b10  general
//         0x3ffbxxxx -- 0b11  to_host
#define SLC_DATA_ADDR_CLEAR_MASK                    (~(0xf<<12)) 
#define SLC_FROM_HOST_ADDR_MASK                     (0x1<<12)
#define SLC_TO_HOST_ADDR_MASK                       (0x3<<12)

#define SLC_SET_FROM_HOST_ADDR_MASK(v)   do { \
    (v) &= SLC_DATA_ADDR_CLEAR_MASK;  \
    (v) |= SLC_FROM_HOST_ADDR_MASK;   \
} while(0);

#define SLC_SET_TO_HOST_ADDR_MASK(v)   do { \
    (v) &= SLC_DATA_ADDR_CLEAR_MASK;  \
    (v) |= SLC_TO_HOST_ADDR_MASK;   \
} while(0);


#endif

