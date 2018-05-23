#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "edison-9dof-i2c.h"

const float GyroScaleValue[] = {
  245 / 32768.0,
  500 / 32768.0,
  2000 / 32768.0
};

const float AccelScaleValue[] = {
  2 / 32768.0,
  4 / 32768.0,
  6 / 32768.0,
  8 / 32768.0,
  16 / 32768.0
};
const float MagScaleValue[] = {
  2 / 32768.0,
  4 / 32768.0,
  6 / 32768.0,
  8 / 32768.0,
  12 / 32768.0
};

int init_device (const char* device_name)
{
  int file;
  uint8_t g_id, xm_id;

  if ((file = open(device_name, O_RDWR)) < 0) {
    fprintf(stderr, "Failed to open the i2c bus '%s'\n", device_name);
    return 0;
  }

  read_byte (file, G_ADDRESS, WHO_AM_I_G, &g_id);
  read_byte (file, XM_ADDRESS, WHO_AM_I_XM, &xm_id);
  if (g_id != 0xD4 || xm_id != 0x49) {
    fprintf(stderr, "Device id mismatch: Got %02x/%02x, expected %02x/%02x\n",
            g_id, xm_id, 0xD4, 0x49);
    close (file);
    return 0;
  }

  return file;
}

int write_bytes (int file, uint8_t address, uint8_t *data, uint8_t count)
{
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[1];

  messages[0].addr  = address;
  messages[0].flags = 0;
  messages[0].len   = count;
  messages[0].buf   = data;

  packets.msgs      = messages;
  packets.nmsgs     = 1;

  return ioctl(file, I2C_RDWR, &packets) >= 0 ;
}

int write_byte (int file, uint8_t address, uint8_t reg, uint8_t data)
{
  uint8_t buf[2];
  buf[0] = reg;
  buf[1] = data;
  return write_bytes (file, address, buf, 2);
}

int read_bytes (int file, uint8_t address, uint8_t reg, uint8_t *dest, uint8_t count)
{
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[2];

  /* secret handshake for multibyte read */
  reg = reg | 0x80;

  /* write the register we want to read from */
  messages[0].addr  = address;
  messages[0].flags = 0;
  messages[0].len   = 1;
  messages[0].buf   = &reg;

  /* read */
  messages[1].addr  = address;
  messages[1].flags = I2C_M_RD;
  messages[1].len   = count;
  messages[1].buf   = dest;

  packets.msgs      = messages;
  packets.nmsgs     = 2;

  return ioctl(file, I2C_RDWR, &packets) >= 0;
}

int read_byte (int file, uint8_t address, uint8_t reg, uint8_t *dest)
{
  return read_bytes (file, address, reg, dest, 1);
}

int read_triplet (int file, uint8_t address, uint8_t reg, Triplet *coords)
{
  uint8_t data[6] = {0};
  int retval;

  retval = read_bytes (file, address, reg, &data[0], 6);
  coords->x = ((data[1] << 8) | data[0]);
  coords->y = ((data[3] << 8) | data[2]);
  coords->z = ((data[5] << 8) | data[4]);
  return retval;
}

int read_gyro (int file, Triplet g_bias, GyroScale scale, FTriplet *dps)
{
  Triplet data = {0};
  int retval;

  retval = read_triplet (file, G_ADDRESS, OUT_X_L_G, &data);
  dps->x = (data.x - g_bias.x) * GyroScaleValue[scale];
  dps->y = (data.y - g_bias.y) * GyroScaleValue[scale];
  dps->z = (data.z - g_bias.z) * GyroScaleValue[scale];

  return retval;
}

int read_mag (int file, Triplet m_bias, FTriplet m_scale, MagScale scale, FTriplet *gauss)
{
  Triplet data = {0};
  int retval;

  retval = read_triplet (file, XM_ADDRESS, OUT_X_L_M, &data);

  gauss->x = (data.x - m_bias.x) * m_scale.x * MagScaleValue[scale];
  gauss->y = (data.y - m_bias.y) * m_scale.y * MagScaleValue[scale];
  /* invert z axis so it's positive down like other sensors */
  gauss->z = -(data.z - m_bias.z) * m_scale.z * MagScaleValue[scale];

  return retval;
}

int read_acc (int file, Triplet a_bias, AccelScale scale, FTriplet *grav)
{
  Triplet data = {0};
  int retval;

  retval = read_triplet (file, XM_ADDRESS, OUT_X_L_A, &data);

  grav->x = (data.x - a_bias.x) * AccelScaleValue[scale];
  grav->y = (data.y - a_bias.y) * AccelScaleValue[scale];
  grav->z = (data.z - a_bias.z) * AccelScaleValue[scale];

  return retval;
}

void init_gyro (int file, GyroScale scale)
{
  uint8_t reg4_g;

  // normal mode, all axes
  write_byte (file, G_ADDRESS, CTRL_REG1_G, 0x0f);

  // zero the scale bits, then set them
  read_byte (file, G_ADDRESS,  CTRL_REG4_G, &reg4_g);
  write_byte (file, G_ADDRESS, CTRL_REG4_G, (reg4_g & 0xCF) | scale << 4);
}

void init_mag (int file, MagScale scale)
{
  // enable temp sensor
  write_byte (file, XM_ADDRESS, CTRL_REG5_XM, 0x98);

  // all other bits 0
  write_byte (file, XM_ADDRESS, CTRL_REG6_XM, scale << 5);

  // continuous conversion mode
  write_byte (file, XM_ADDRESS, CTRL_REG7_XM, 0x00);
}

void init_acc (int file, AccelScale scale)
{
  uint8_t reg2_xm;

  // 100hz, all axes
  write_byte (file, XM_ADDRESS, CTRL_REG1_XM, 0x57);

  // zero the scale bits, then set them
  read_byte (file, XM_ADDRESS, CTRL_REG2_XM, &reg2_xm);
  write_byte (file, XM_ADDRESS, CTRL_REG2_XM, (reg2_xm & 0xC7) | scale << 3);
}
/*  

//define BYTE2BIN(byte) \
//   (byte & 0x80 ? 1 : 0), \
//    (byte & 0x40 ? 1 : 0), \
//    (byte & 0x20 ? 1 : 0), \
//    (byte & 0x10 ? 1 : 0), \
//    (byte & 0x08 ? 1 : 0), \
//    (byte & 0x04 ? 1 : 0), \
//    (byte & 0x02 ? 1 : 0), \
//    (byte & 0x01 ? 1 : 0)

//uint8_t print_byte;
//define PRINT_REGISTER(file, address, reg) \
//  read_byte(file, address, reg, &print_byte); \
//  printf ("%-18s\t%02x / %d%d%d%d%d%d%d%d\n", \
//          #reg":", print_byte, BYTE2BIN(print_byte))
//void dump_config_registers (int file)
//{
//  printf (" * Non-output registers for %02x:\n", G_ADDRESS);
//  PRINT_REGISTER (file, G_ADDRESS, WHO_AM_I_XM);
//  PRINT_REGISTER (file, G_ADDRESS, CTRL_REG1_XM);
//  PRINT_REGISTER (file, G_ADDRESS, CTRL_REG2_XM);
//  PRINT_REGISTER (file, G_ADDRESS, CTRL_REG3_XM);
//  PRINT_REGISTER (file, G_ADDRESS, CTRL_REG4_XM);
//  PRINT_REGISTER (file, G_ADDRESS, CTRL_REG5_XM);
//  PRINT_REGISTER (file, G_ADDRESS, REFERENCE_G);
///  PRINT_REGISTER (file, G_ADDRESS, FIFO_CTRL_REG_G);
//  PRINT_REGISTER (file, G_ADDRESS, INT1_CFG_G);
//  PRINT_REGISTER (file, G_ADDRESS, INT1_TSH_XH_G);
//  PRINT_REGISTER (file, G_ADDRESS, INT1_TSH_XL_G);
//  PRINT_REGISTER (file, G_ADDRESS, INT1_TSH_YH_G);
//  PRINT_REGISTER (file, G_ADDRESS, INT1_TSH_YL_G);
//  PRINT_REGISTER (file, G_ADDRESS, INT1_TSH_ZH_G);
//  PRINT_REGISTER (file, G_ADDRESS, INT1_TSH_ZL_G);
//  PRINT_REGISTER (file, G_ADDRESS, INT1_DURATION_G);

//  printf (" * Non-output registers for %02x:\n", XM_ADDRESS);
//  PRINT_REGISTER (file, XM_ADDRESS, WHO_AM_I_XM);
//  PRINT_REGISTER (file, XM_ADDRESS, INT_CTRL_REG_M);
//  PRINT_REGISTER (file, XM_ADDRESS, INT_THS_L_M);
//  PRINT_REGISTER (file, XM_ADDRESS, INT_THS_H_M);
//  PRINT_REGISTER (file, XM_ADDRESS, OFFSET_X_L_M);
///  PRINT_REGISTER (file, XM_ADDRESS, OFFSET_X_H_M);
//  PRINT_REGISTER (file, XM_ADDRESS, OFFSET_Y_L_M);
//  PRINT_REGISTER (file, XM_ADDRESS, OFFSET_Y_H_M);
//  PRINT_REGISTER (file, XM_ADDRESS, OFFSET_Z_L_M);
//  PRINT_REGISTER (file, XM_ADDRESS, OFFSET_Z_H_M);
//  PRINT_REGISTER (file, XM_ADDRESS, REFERENCE_X);
//  PRINT_REGISTER (file, XM_ADDRESS, REFERENCE_Y);
// PRINT_REGISTER (file, XM_ADDRESS, REFERENCE_Z);
//  PRINT_REGISTER (file, XM_ADDRESS, CTRL_REG0_XM);
//  PRINT_REGISTER (file, XM_ADDRESS, CTRL_REG1_XM);
//  PRINT_REGISTER (file, XM_ADDRESS, CTRL_REG2_XM);
//  PRINT_REGISTER (file, XM_ADDRESS, CTRL_REG3_XM);
//  PRINT_REGISTER (file, XM_ADDRESS, CTRL_REG4_XM);
//  PRINT_REGISTER (file, XM_ADDRESS, CTRL_REG5_XM);
// PRINT_REGISTER (file, XM_ADDRESS, CTRL_REG6_XM);
//  PRINT_REGISTER (file, XM_ADDRESS, CTRL_REG7_XM);
//  PRINT_REGISTER (file, XM_ADDRESS, FIFO_CTRL_REG);
//  PRINT_REGISTER (file, XM_ADDRESS, INT_GEN_1_REG);
//  PRINT_REGISTER (file, XM_ADDRESS, INT_GEN_1_THS);
//  PRINT_REGISTER (file, XM_ADDRESS, INT_GEN_1_DURATION);
//  PRINT_REGISTER (file, XM_ADDRESS, INT_GEN_2_REG);
//  PRINT_REGISTER (file, XM_ADDRESS, INT_GEN_2_THS);
//  PRINT_REGISTER (file, XM_ADDRESS, INT_GEN_2_DURATION);
//  PRINT_REGISTER (file, XM_ADDRESS, CLICK_CFG);
//  PRINT_REGISTER (file, XM_ADDRESS, CLICK_THS);
//  PRINT_REGISTER (file, XM_ADDRESS, TIME_LIMIT);
//  PRINT_REGISTER (file, XM_ADDRESS, TIME_LATENCY);
//  PRINT_REGISTER (file, XM_ADDRESS, TIME_WINDOW);
//  PRINT_REGISTER (file, XM_ADDRESS, ACT_THS);
//  PRINT_REGISTER (file, XM_ADDRESS, ACT_DUR);
//}
*/

int16_t getTemp(int i2c_handle)
{
    uint8_t data[2] = {0};
    read_bytes (i2c_handle, XM_ADDRESS, OUT_TEMP_L_XM, &data[0], 2);
    int16_t temp = (((data[1] & 0x0f) << 8) | data[0]);
    return temp;
}
