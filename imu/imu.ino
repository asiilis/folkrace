/*
Brian R Taylor
brian.taylor@bolderflight.com
2016-10-10

Copyright (c) 2016 Bolder Flight Systems

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Arduino.h"
#include "i2c_t3.h"

enum mpu9250_gyro_range
{
    GYRO_RANGE_250DPS,
    GYRO_RANGE_500DPS,
    GYRO_RANGE_1000DPS,
    GYRO_RANGE_2000DPS
};

enum mpu9250_accel_range
{
    ACCEL_RANGE_2G,
    ACCEL_RANGE_4G,
    ACCEL_RANGE_8G,
    ACCEL_RANGE_16G
};

enum mpu9250_dlpf_bandwidth
{
    DLPF_BANDWIDTH_184HZ,
    DLPF_BANDWIDTH_92HZ,
    DLPF_BANDWIDTH_41HZ,
    DLPF_BANDWIDTH_20HZ,
    DLPF_BANDWIDTH_10HZ,
    DLPF_BANDWIDTH_5HZ
};

class MPU9250
{
    public:
        MPU9250(uint8_t address, uint8_t bus, i2c_pins pins, i2c_pullup pullups);

        int begin(mpu9250_accel_range accelRange, mpu9250_gyro_range gyroRange);
        int setFilter(mpu9250_dlpf_bandwidth bandwidth, uint8_t srd);

        void MadgwickQuaternionUpdate(float* data, float t);
        void toEulerianAngle(float* q, float& roll, float& pitch, float& yaw);

        void getMotion10Unbiased(float* res);
        void getMotion10(float* res);
        void getMotion10Counts(int16_t* res);

        void setBias(float* b);

        // constants
        const float G = 9.807f;
        const float DEG2RAD = 3.14159265359f/180.0f;
        const float RAD2DEG = 180.0f/3.14159265359f;

        float delta = 0.0f;

    private:

        // I2C address
        uint8_t _address;
        uint8_t _bus;
        i2c_pins _pins;
        i2c_pullup _pullups;

        // MPU9250 sensitivity
        float _accelScale;
        float _gyroScale;

        // MPU9250 magnetometer pre-scaler
        float _magScaleX, _magScaleY, _magScaleZ;

        // biases
        float bias[18] = {
            0.0f, 0.0f, 0.0f, // accel bias
            0.0f, 0.0f, 0.0f, // gyro bias
            0.0f, 0.0f, 0.0f, // megnetometer bias

            // magnetometer scale matrix
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
        };

        // timings
        unsigned long update = 0L;

        // data indices
        const int AX = 0;
        const int AY = 1;
        const int AZ = 2;

        const int GX = 3;
        const int GY = 4;
        const int GZ = 5;

        const int MX = 6;
        const int MY = 7;
        const int MZ = 8;

        const int TEMPERATURE = 9;

        // quaternion indices
        const int QW = 0;
        const int QX = 1;
        const int QY = 2;
        const int QZ = 3;

        // megnetometer scale bias indices
        // data indices
        const int MXX = 9;
        const int MXY = 10;
        const int MXZ = 11;

        const int MYX = 12;
        const int MYY = 13;
        const int MYZ = 14;

        const int MZX = 15;
        const int MZY = 16;
        const int MZZ = 17;

        // temperature constants
        const float TEMPERATURE_SCALE = 1.0f/333.87f;
        const float TEMPERATURE_OFFSET = 21.0f;

        // i2c bus frequency
        const uint32_t I2C_RATE = 400000;

        // MPU9250 registers
        const uint8_t ACCEL_OUT = 0x3B;
        const uint8_t GYRO_OUT = 0x43;
        const uint8_t TEMP_OUT = 0x41;
        const uint8_t EXT_SENS_DATA_00 = 0x49;

        const uint8_t ACCEL_CONFIG = 0x1C;
        const uint8_t ACCEL_FS_SEL_2G = 0x00;
        const uint8_t ACCEL_FS_SEL_4G = 0x08;
        const uint8_t ACCEL_FS_SEL_8G = 0x10;
        const uint8_t ACCEL_FS_SEL_16G = 0x18;

        const uint8_t GYRO_CONFIG = 0x1B;
        const uint8_t GYRO_FS_SEL_250DPS = 0x00;
        const uint8_t GYRO_FS_SEL_500DPS = 0x08;
        const uint8_t GYRO_FS_SEL_1000DPS = 0x10;
        const uint8_t GYRO_FS_SEL_2000DPS = 0x18;

        const uint8_t ACCEL_CONFIG2 = 0x1D;
        const uint8_t ACCEL_DLPF_184 = 0x01;
        const uint8_t ACCEL_DLPF_92 = 0x02;
        const uint8_t ACCEL_DLPF_41 = 0x03;
        const uint8_t ACCEL_DLPF_20 = 0x04;
        const uint8_t ACCEL_DLPF_10 = 0x05;
        const uint8_t ACCEL_DLPF_5 = 0x06;

        const uint8_t CONFIG = 0x1A;
        const uint8_t GYRO_DLPF_184 = 0x01;
        const uint8_t GYRO_DLPF_92 = 0x02;
        const uint8_t GYRO_DLPF_41 = 0x03;
        const uint8_t GYRO_DLPF_20 = 0x04;
        const uint8_t GYRO_DLPF_10 = 0x05;
        const uint8_t GYRO_DLPF_5 = 0x06;

        const uint8_t SMPDIV = 0x19;

        const uint8_t INT_PIN_CFG = 0x37;
        const uint8_t INT_ENABLE = 0x38;
        const uint8_t INT_PULSE_50US = 0x00;
        const uint8_t INT_RAW_RDY_EN = 0x01;

        const uint8_t PWR_MGMNT_1 = 0x6B;
        const uint8_t PWR_RESET = 0x80;
        const uint8_t CLOCK_SEL_PLL = 0x01;

        const uint8_t PWR_MGMNT_2 = 0x6C;
        const uint8_t SEN_ENABLE = 0x00;

        const uint8_t USER_CTRL = 0x6A;
        const uint8_t I2C_MST_EN = 0x20;
        const uint8_t I2C_MST_CLK = 0x0D;
        const uint8_t I2C_MST_CTRL = 0x24;
        const uint8_t I2C_SLV0_ADDR = 0x25;
        const uint8_t I2C_SLV0_REG = 0x26;
        const uint8_t I2C_SLV0_DO = 0x63;
        const uint8_t I2C_SLV0_CTRL = 0x27;
        const uint8_t I2C_SLV0_EN = 0x80;
        const uint8_t I2C_READ_FLAG = 0x80;

        const uint8_t WHO_AM_I = 0x75;

        // AK8963 registers
        const uint8_t AK8963_I2C_ADDR = 0x0C;

        const uint8_t AK8963_HXL = 0x03;

        const uint8_t AK8963_CNTL1 = 0x0A;
        const uint8_t AK8963_PWR_DOWN = 0x00;
        const uint8_t AK8963_CNT_MEAS1 = 0x12;
        const uint8_t AK8963_CNT_MEAS2 = 0x16;
        const uint8_t AK8963_FUSE_ROM = 0x0F;

        const uint8_t AK8963_CNTL2 = 0x0B;
        const uint8_t AK8963_RESET = 0x01;

        const uint8_t AK8963_ASA = 0x10;

        const uint8_t AK8963_WHO_AM_I = 0x00;

        // communication helpers
        bool writeRegister(uint8_t subAddress, uint8_t data);
        void readRegisters(uint8_t subAddress, uint8_t count, uint8_t* dest);
        bool writeAK8963Register(uint8_t subAddress, uint8_t data);
        void readAK8963Registers(uint8_t subAddress, uint8_t count, uint8_t* dest);

        uint8_t whoAmI();
        uint8_t whoAmIAK8963();
};

/* MPU9250 object, input the I2C address, I2C bus, I2C pins, and I2C pullups */
MPU9250::MPU9250(uint8_t address, uint8_t bus, i2c_pins pins, i2c_pullup pullups)
{
    _address = address; // I2C address
    _bus = bus; // I2C bus
    _pins = pins; // I2C pins
    _pullups = pullups; // I2C pullups
}

/* starts I2C communication and sets up the MPU-9250 */
int MPU9250::begin(mpu9250_accel_range accelRange, mpu9250_gyro_range gyroRange)
{
    uint8_t buff[3];
    uint8_t data[7];

    uint8_t accelConfig;
    uint8_t gyroConfig;

    // starting the I2C bus
    i2c_t3(_bus).begin(I2C_MASTER, 0, _pins, _pullups, I2C_RATE);

    // enable I2C master mode
    if (!writeRegister(USER_CTRL, I2C_MST_EN))
    {
        return -1;
    }

    // set the I2C bus speed to 400 kHz
    if (!writeRegister(I2C_MST_CTRL, I2C_MST_CLK))
    {
        return -1;
    }

    // set AK8963 to Power Down
    if (!writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN))
    {
        return -1;
    }

    // reset the MPU9250
    writeRegister(PWR_MGMNT_1, PWR_RESET);

    // wait for MPU-9250 to come back up
    delay(1);

    // reset the AK8963
    writeAK8963Register(AK8963_CNTL2, AK8963_RESET);

    // select clock source to gyro
    if (!writeRegister(PWR_MGMNT_1, CLOCK_SEL_PLL))
    {
        return -1;
    }

    // check the WHO AM I byte, expected value is 0x71 (decimal 113)
    if (whoAmI() != 0x71)
    {
        return -1;
    }

    // enable accelerometer and gyro
    if (!writeRegister(PWR_MGMNT_2, SEN_ENABLE))
    {
        return -1;
    }

    // setup the accel and gyro ranges
    switch (accelRange)
    {
        case ACCEL_RANGE_2G:
            // setting the accel range to 2G
            _accelScale = G * 2.0f/32767.5f;
            accelConfig = ACCEL_FS_SEL_2G;
            break;

        case ACCEL_RANGE_4G:
            // setting the accel range to 4G
            _accelScale = G * 4.0f/32767.5f;
            accelConfig = ACCEL_FS_SEL_4G;
            break;

        case ACCEL_RANGE_8G:
            // setting the accel range to 8G
            _accelScale = G * 8.0f/32767.5f;
            accelConfig = ACCEL_FS_SEL_8G;
            break;

        case ACCEL_RANGE_16G:
            // setting the accel range to 16G
            _accelScale = G * 16.0f/32767.5f;
            accelConfig = ACCEL_FS_SEL_16G;
            break;

        default:
            return -1;
    }

    if (!writeRegister(ACCEL_CONFIG, accelConfig))
    {
        return -1;
    }

    switch (gyroRange)
    {
        case GYRO_RANGE_250DPS:
            // setting the gyro range to 250DPS
            _gyroScale = 250.0f/32767.5f * DEG2RAD;
            gyroConfig = GYRO_FS_SEL_250DPS;
            break;

        case GYRO_RANGE_500DPS:
            // setting the gyro range to 500DPS
            _gyroScale = 500.0f/32767.5f * DEG2RAD;
            gyroConfig = GYRO_FS_SEL_500DPS;
            break;

        case GYRO_RANGE_1000DPS:
            // setting the gyro range to 1000DPS
            _gyroScale = 1000.0f/32767.5f * DEG2RAD;
            gyroConfig = GYRO_FS_SEL_1000DPS;
            break;

        case GYRO_RANGE_2000DPS:
            // setting the gyro range to 2000DPS
            _gyroScale = 2000.0f/32767.5f * DEG2RAD;
            gyroConfig = GYRO_FS_SEL_2000DPS;
            break;

        default:
            return -1;
    }

    if (!writeRegister(GYRO_CONFIG, gyroConfig))
    {
        return -1;
    }

    // enable I2C master mode
    if (!writeRegister(USER_CTRL, I2C_MST_EN))
    {
        return -1;
    }

    // set the I2C bus speed to 400 kHz
    if (!writeRegister(I2C_MST_CTRL, I2C_MST_CLK))
    {
        return -1;
    }

    // check AK8963 WHO AM I register, expected value is 0x48 (decimal 72)
    if (whoAmIAK8963() != 0x48)
    {
        return -1;
    }

    /* get the magnetometer calibration */

    // set AK8963 to Power Down
    if (!writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN))
    {
        return -1;
    }
    delay(100); // long wait between AK8963 mode changes

    // set AK8963 to FUSE ROM access
    if (!writeAK8963Register(AK8963_CNTL1, AK8963_FUSE_ROM))
    {
        return -1;
    }
    delay(100); // long wait between AK8963 mode changes

    // read the AK8963 ASA registers and compute magnetometer scale factors
    readAK8963Registers(AK8963_ASA, sizeof(buff), &buff[0]);
    _magScaleX = ((((float)buff[0]) - 128.0f)/(256.0f) + 1.0f) * 4912.0f / 32760.0f; // micro Tesla
    _magScaleY = ((((float)buff[1]) - 128.0f)/(256.0f) + 1.0f) * 4912.0f / 32760.0f; // micro Tesla
    _magScaleZ = ((((float)buff[2]) - 128.0f)/(256.0f) + 1.0f) * 4912.0f / 32760.0f; // micro Tesla

    // set AK8963 to Power Down
    if (!writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN))
    {
        return -1;
    }
    delay(100); // long wait between AK8963 mode changes

    // set AK8963 to 16 bit resolution, 100 Hz update rate
    if (!writeAK8963Register(AK8963_CNTL1, AK8963_CNT_MEAS2))
    {
        return -1;
    }
    delay(100); // long wait between AK8963 mode changes

    // select clock source to gyro
    if (!writeRegister(PWR_MGMNT_1, CLOCK_SEL_PLL))
    {
        return -1;
    }

    // instruct the MPU9250 to get 7 bytes of data from the AK8963 at the sample rate
    readAK8963Registers(AK8963_HXL, sizeof(data), &data[0]);

    // reset timing
    update = 0L;
    delta = 0.0f;

    // successful init, return 0
    return 0;
}


/* sets the DLPF and interrupt settings */
int MPU9250::setFilter(mpu9250_dlpf_bandwidth bandwidth, uint8_t srd)
{
    uint8_t data[7];

    uint8_t accelBandwidth;
    uint8_t gyroBandwidth;

    switch (bandwidth)
    {
        case DLPF_BANDWIDTH_184HZ:
            accelBandwidth = ACCEL_DLPF_184;
            gyroBandwidth = GYRO_DLPF_184;
            break;

        case DLPF_BANDWIDTH_92HZ:
            accelBandwidth = ACCEL_DLPF_92;
            gyroBandwidth = GYRO_DLPF_92;
            break;

        case DLPF_BANDWIDTH_41HZ:
            accelBandwidth = ACCEL_DLPF_41;
            gyroBandwidth = GYRO_DLPF_41;
            break;

        case DLPF_BANDWIDTH_20HZ:
            accelBandwidth = ACCEL_DLPF_20;
            gyroBandwidth = GYRO_DLPF_20;
            break;

        case DLPF_BANDWIDTH_10HZ:
            accelBandwidth = ACCEL_DLPF_5;
            gyroBandwidth = GYRO_DLPF_5;
            break;

        case DLPF_BANDWIDTH_5HZ:
            accelBandwidth = ACCEL_DLPF_20;
            gyroBandwidth = GYRO_DLPF_20;
            break;

        default:
            return -1;
    }

    // setting accel bandwidth
    if (!writeRegister(ACCEL_CONFIG2, accelBandwidth))
    {
        return -1;
    }

    // setting gyro bandwidth
    if (!writeRegister(CONFIG, gyroBandwidth))
    {
        return -1;
    }

    // setting the sample rate divider
    if (!writeRegister(SMPDIV, srd))
    {
        return -1;
    }

    if (srd > 9)
    {

        // set AK8963 to Power Down
        if (!writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN))
        {
            return -1;
        }
        delay(100); // long wait between AK8963 mode changes

        // set AK8963 to 16 bit resolution, 8 Hz update rate
        if (!writeAK8963Register(AK8963_CNTL1, AK8963_CNT_MEAS1))
        {
            return -1;
        }
        delay(100); // long wait between AK8963 mode changes

        // instruct the MPU9250 to get 7 bytes of data from the AK8963 at the sample rate
        readAK8963Registers(AK8963_HXL, sizeof(data), &data[0]);
    }

    // setup interrupt, 50 us pulse
    if (!writeRegister(INT_PIN_CFG, INT_PULSE_50US))
    { //
        return -1;
    }

    if (!writeRegister(INT_ENABLE, INT_RAW_RDY_EN))
    { // set to data ready
        return -1;
    }

    // successful filter setup, return 0
    return 0;
}

/* get accelerometer, gyro, magnetometer, and temperature data given pointers to store values, return data as counts */
void MPU9250::getMotion10Counts(int16_t* res)
{
    uint8_t buff[21];
    // this will also read AK8963_ST2 0x09 - Data overflow bit 3 and data read error status bit 2
    // via EXT_SENS_DATA_07 0x50 register
    // not checking it now, but probably should

    readRegisters(ACCEL_OUT, sizeof(buff), &buff[0]); // grab the data from the MPU9250

    // Sensors x (y)-axis of the accelerometer/gyro is aligned with the y (x)-axis of the magnetometer;
    // the magnetometer z-axis (+ down) is misaligned with z-axis (+ up) of accelerometer and gyro!
    // We have to make some allowance for this orientation mismatch in feeding the output to the quaternion filter.
    // For the MPU9250+MS5637 Mini breakout the +x accel/gyro is North, then -y accel/gyro is East. So if we want te quaternions properly aligned
    // we need to feed into the Madgwick function Ax, -Ay, -Az, Gx, -Gy, -Gz, My, -Mx, and Mz. But because gravity is by convention
    // positive down, we need to invert the accel data, so we pass -Ax, Ay, Az, Gx, -Gy, -Gz, My, -Mx, and Mz into the Madgwick
    // function to get North along the accel +x-axis, East along the accel -y-axis, and Down along the accel -z-axis.
    // This orientation choice can be modified to allow any convenient (non-NED) orientation convention.

    // combine into 16 bit values
    // accel/gyro is arranged HIGH, then LOW
    res[AX] = -((((int16_t)buff[0]) << 8) | buff[1]);
    res[AY] =  ((((int16_t)buff[2]) << 8) | buff[3]);
    res[AZ] =  ((((int16_t)buff[4]) << 8) | buff[5]);

    res[GX] =  ((((int16_t)buff[8]) << 8) | buff[9]);
    res[GY] = -((((int16_t)buff[10]) << 8) | buff[11]);
    res[GZ] = -((((int16_t)buff[12]) << 8) | buff[13]);

    // note exchange of indices 7 (hy) and 6 (hx)
    // magnetometer is arranged LOW, then HIGH
    res[MY] =  ((((int16_t)buff[15]) << 8) | buff[14]);
    res[MX] = -((((int16_t)buff[17]) << 8) | buff[16]);
    res[MZ] =  ((((int16_t)buff[19]) << 8) | buff[18]);

    res[TEMPERATURE] = ((((int16_t)buff[6]) << 8) | buff[7]);

    float us = micros();
    delta = (float)(us - update) * 0.000001; // uS -> full seconds
    update = us;
}

void MPU9250::getMotion10(float* res)
{
    int16_t buff[10];

    getMotion10Counts(buff);

    // typecast and scale to values
    res[AX] = ((float) buff[AX]) * _accelScale;
    res[AY] = ((float) buff[AY]) * _accelScale;
    res[AZ] = ((float) buff[AZ]) * _accelScale;

    res[GX] = ((float) buff[GX]) * _gyroScale;
    res[GY] = ((float) buff[GY]) * _gyroScale;
    res[GZ] = ((float) buff[GZ]) * _gyroScale;

    res[MX] = ((float) buff[MX]) * _magScaleY; // y sacle - see XY swap
    res[MY] = ((float) buff[MY]) * _magScaleX; // x scale - see XY swap
    res[MZ] = ((float) buff[MZ]) * _magScaleZ;

    res[TEMPERATURE] = ((((float) buff[TEMPERATURE]) - TEMPERATURE_OFFSET) * TEMPERATURE_SCALE) + TEMPERATURE_OFFSET; // t
}

void MPU9250::getMotion10Unbiased(float* res)
{
    getMotion10(res);

    // typecast and scale to values
    res[AX] -= bias[AX];
    res[AY] -= bias[AY];
    res[AZ] -= bias[AZ];

    res[GX] -= bias[GX];
    res[GY] -= bias[GY];
    res[GZ] -= bias[GZ];

    float cmx = res[MX] - bias[MX];
    float cmy = res[MY] - bias[MY];
    float cmz = res[MZ] - bias[MZ];

    res[MX] = cmx*bias[MXX] + cmy*bias[MXY] + cmz*bias[MXZ];
    res[MY] = cmx*bias[MYX] + cmy*bias[MYY] + cmz*bias[MYZ];
    res[MZ] = cmx*bias[MZX] + cmy*bias[MZY] + cmz*bias[MZZ];

    // no bias for temperature
}

void MPU9250::setBias(float* b)
{
    for (int i=0; i<18; i++)
    {
        bias[i] = b[i];
    }
}


/* writes a byte to MPU9250 register given a register address and data */
bool MPU9250::writeRegister(uint8_t subAddress, uint8_t data)
{
    uint8_t buff[1];

    i2c_t3(_bus).beginTransmission(_address); // open the device
    i2c_t3(_bus).write(subAddress); // write the register address
    i2c_t3(_bus).write(data); // write the data
    i2c_t3(_bus).endTransmission();
    delay(10); // need to slow down how fast I write to MPU9250

    // read back the register
    readRegisters(subAddress, sizeof(buff), &buff[0]);

    // check the read back register against the written register
    if (buff[0] == data) {
        return true;
    }
    else
    {
        return false;
    }
}

/* reads registers from MPU9250 given a starting register address, number of bytes, and a pointer to store data */
void MPU9250::readRegisters(uint8_t subAddress, uint8_t count, uint8_t* dest)
{
    i2c_t3(_bus).beginTransmission(_address); // open the device
    i2c_t3(_bus).write(subAddress); // specify the starting register address
    i2c_t3(_bus).endTransmission(false);

    i2c_t3(_bus).requestFrom(_address, count); // specify the number of bytes to receive

    while (i2c_t3(_bus).available())
    {
        *dest++ = i2c_t3(_bus).readByte();
    }
}

/* writes a register to the AK8963 given a register address and data */
bool MPU9250::writeAK8963Register(uint8_t subAddress, uint8_t data)
{
    uint8_t buff[1];

    writeRegister(I2C_SLV0_ADDR, AK8963_I2C_ADDR); // set slave 0 to the AK8963 and set for write
    writeRegister(I2C_SLV0_REG, subAddress); // set the register to the desired AK8963 sub address
    writeRegister(I2C_SLV0_DO, data); // store the data for write
    writeRegister(I2C_SLV0_CTRL, I2C_SLV0_EN | 1); // enable I2C and send 1 byte

    // read the register and confirm
    readAK8963Registers(subAddress, sizeof(buff), &buff[0]);

    if (buff[0] == data)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/* reads registers from the AK8963 */
void MPU9250::readAK8963Registers(uint8_t subAddress, uint8_t count, uint8_t* dest)
{
    writeRegister(I2C_SLV0_ADDR, AK8963_I2C_ADDR | I2C_READ_FLAG); // set slave 0 to the AK8963 and set for read
    writeRegister(I2C_SLV0_REG, subAddress); // set the register to the desired AK8963 sub address
    writeRegister(I2C_SLV0_CTRL, I2C_SLV0_EN | count); // enable I2C and request the bytes
    delayMicroseconds(100); // takes some time for these registers to fill
    readRegisters(EXT_SENS_DATA_00, count, dest); // read the bytes off the MPU9250 EXT_SENS_DATA registers
}

/* gets the MPU9250 WHO_AM_I register value, expected to be 0x71 */
uint8_t MPU9250::whoAmI()
{
    uint8_t buff[1];

    // read the WHO AM I register
    readRegisters(WHO_AM_I, sizeof(buff), &buff[0]);

    // return the register value
    return buff[0];
}

/* gets the AK8963 WHO_AM_I register value, expected to be 0x48 */
uint8_t MPU9250::whoAmIAK8963()
{
    uint8_t buff[1];

    // read the WHO AM I register
    readAK8963Registers(AK8963_WHO_AM_I, sizeof(buff), &buff[0]);

    // return the register value
    return buff[0];
}

/////////////////////////////////

// These are the free parameters in the Mahony filter and fusion scheme, Kp
// for proportional feedback, Ki for integral
#define Kp 2.0f * 5.0f
#define Ki 0.0f

static float GyroMeasError = PI * (40.0f / 180.0f);

// There is a tradeoff in the beta parameter between accuracy and response
// speed. In the original Madgwick study, beta of 0.041 (corresponding to
// GyroMeasError of 2.7 degrees/s) was found to give optimal accuracy.
// However, with this value, the LSM9SD0 response time is about 10 seconds
// to a stable initial quaternion. Subsequent changes also require a
// longish lag time to a stable output, not fast enough for a quadcopter or
// robot car! By increasing beta (GyroMeasError) by about a factor of
// fifteen, the response time constant is reduced to ~2 sec. I haven't
// noticed any reduction in solution accuracy. This is essentially the I
// coefficient in a PID control sense; the bigger the feedback coefficient,
// the faster the solution converges, usually at the expense of accuracy.
// In any case, this is the free parameter in the Madgwick filtering and
// fusion scheme.
static float beta = sqrt(3.0f / 4.0f) * GyroMeasError;   // Compute beta

// Vector to hold quaternion
static float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};

void  MPU9250::MadgwickQuaternionUpdate(float* data, float t)
{
    if (t == 0.0f) return; // handle no timing => no update

    // short name local variable for readability
    float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];
    float norm;
    float hx, hy, _2bx, _2bz;
    float s1, s2, s3, s4;
    float qDot1, qDot2, qDot3, qDot4;

    // Auxiliary variables to avoid repeated arithmetic
    float _2q1mx;
    float _2q1my;
    float _2q1mz;
    float _2q2mx;
    float _4bx;
    float _4bz;

    float q1q1 = q1 * q1;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q1q4 = q1 * q4;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q2q4 = q2 * q4;
    float q3q3 = q3 * q3;
    float q3q4 = q3 * q4;
    float q4q4 = q4 * q4;

    float _2q1 = 2.0f * q1;
    float _2q2 = 2.0f * q2;
    float _2q3 = 2.0f * q3;
    float _2q4 = 2.0f * q4;

    float _2q1q3 = 2.0f * q1q3;
    float _2q3q4 = 2.0f * q3q4;

    // Normalise accelerometer measurement
    norm = sqrt(data[0] * data[0] + data[1] * data[1] + data[2] * data[2]);
    if (norm == 0.0f) return; // handle NaN
    norm = 1.0f/norm;
    float ax = data[AX] * norm;
    float ay = data[AY] * norm;
    float az = data[AZ] * norm;

    // Normalise magnetometer measurement
    norm = sqrt(data[6] * data[6] + data[7] * data[7] + data[8] * data[8]);
    if (norm == 0.0f) return; // handle NaN
    norm = 1.0f/norm;
    float mx = data[6] * norm;
    float my = data[7] * norm;
    float mz = data[8] * norm;

    // Reference direction of Earth's magnetic field
    _2q1mx = 2.0f * q1 * mx;
    _2q1my = 2.0f * q1 * my;
    _2q1mz = 2.0f * q1 * mz;
    _2q2mx = 2.0f * q2 * mx;
    hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
    hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
    _2bx = sqrt(hx * hx + hy * hy);
    _2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
    _4bx = 2.0f * _2bx;
    _4bz = 2.0f * _2bz;

    // Gradient decent algorithm corrective step
    s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - ax) + _2q2 * (2.0f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - ax) + _2q1 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q2 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - ax) + _2q4 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q3 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - ax) + _2q3 * (2.0f * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);

    // normalise step magnitude
    norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);
    norm = 1.0f/norm;
    s1 *= norm;
    s2 *= norm;
    s3 *= norm;
    s4 *= norm;

    // short name local variable for readability
    float gx = data[3];
    float gy = data[4];
    float gz = data[5];

    // Compute rate of change of quaternion
    qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1;
    qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy) - beta * s2;
    qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx) - beta * s3;
    qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx) - beta * s4;

    // Integrate to yield quaternion
    q1 += qDot1 * t;
    q2 += qDot2 * t;
    q3 += qDot3 * t;
    q4 += qDot4 * t;

    // normalise quaternion
    norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
    norm = 1.0f/norm;
    q[0] = q1 * norm;
    q[1] = q2 * norm;
    q[2] = q3 * norm;
    q[3] = q4 * norm;
}

/////////////////////////////////

// an MPU9250 object with its I2C address
// of 0x68 (ADDR to GRND) and on Teensy 3.5 bus 0
MPU9250 IMU(0x68, 0, I2C_PINS_18_19, I2C_PULLUP_EXT);

float d[10];
int beginStatus;

void setup()
{
    // serial to display data
    Serial.begin(115200);

    // start communication with IMU and
    // set the accelerometer and gyro ranges.
    // ACCELEROMETER 2G 4G 8G 16G
    // GYRO 250DPS 500DPS 1000DPS 2000DPS
    beginStatus = IMU.begin(ACCEL_RANGE_4G, GYRO_RANGE_500DPS);

    float b[18] = {
            -0.66359285f-0.13f, 0.596398637f, 0.149558031f,  // accel bias
            -0.00336684f, -0.038908617f, -0.003352749f, // gyro bias

            -7.909176574f, 10.11728131f, 22.60300639f, // megnetometer bias

            // magnetometer scale matrix
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
    };

    IMU.setBias(b);
}

void loop()
{
    if (beginStatus < 0)
    {
        delay(1000);
        Serial.println("IMU initialization unsuccessful");
        Serial.println("Check IMU wiring or try cycling power");
        delay(10000);
    }
    else
    {
        // get the accel (m/s/s), gyro (rad/s), and magnetometer (uT), and temperature (C) data
        IMU.getMotion10Unbiased(d);

        // Sensors x (y)-axis of the accelerometer/gyro is aligned with the y (x)-axis of the magnetometer;
        // the magnetometer z-axis (+ down) is misaligned with z-axis (+ up) of accelerometer and gyro!
        // We have to make some allowance for this orientation mismatch in feeding the output to the quaternion filter.
        // For the MPU9250+MS5637 Mini breakout the +x accel/gyro is North, then -y accel/gyro is East. So if we want te quaternions properly aligned
        // we need to feed into the Madgwick function Ax, -Ay, -Az, Gx, -Gy, -Gz, My, -Mx, and Mz. But because gravity is by convention
        // positive down, we need to invert the accel data, so we pass -Ax, Ay, Az, Gx, -Gy, -Gz, My, -Mx, and Mz into the Madgwick
        // function to get North along the accel +x-axis, East along the accel -y-axis, and Down along the accel -z-axis.
        // This orientation choice can be modified to allow any convenient (non-NED) orientation convention.
        // Pass gyro rate as rad/sx

        IMU.MadgwickQuaternionUpdate(d, IMU.delta);

        // print the data
        printData();

        // delay a frame
        delay(50);
    }
}

void printData()
{
    Serial.print(micros());

    Serial.print(" "); Serial.print(d[0], 6);
    Serial.print(" "); Serial.print(d[1], 6);
    Serial.print(" "); Serial.print(d[2], 6);

    Serial.print(" "); Serial.print(d[3], 6);
    Serial.print(" "); Serial.print(d[4], 6);
    Serial.print(" "); Serial.print(d[5], 6);

    Serial.print(" "); Serial.print(d[6], 6);
    Serial.print(" "); Serial.print(d[7], 6);
    Serial.print(" "); Serial.print(d[8], 6);

    Serial.print(" "); Serial.print(q[0], 6);
    Serial.print(" "); Serial.print(q[1], 6);
    Serial.print(" "); Serial.print(q[2], 6);
    Serial.print(" "); Serial.print(q[3], 6);

    float roll;
    float pitch;
    float yaw;

    IMU.toEulerianAngle(q, roll, pitch, yaw);

    Serial.print(" "); Serial.print(roll, 6);
    Serial.print(" "); Serial.print(pitch, 6);
    Serial.print(" "); Serial.print(yaw, 6);

    /*
    Serial.print(" "); Serial.print(d[TEMPERATURE],6); // t
    */

    Serial.println();
}

void MPU9250::toEulerianAngle(float* q, float& roll, float& pitch, float& yaw)
{
    float ysqr = q[QY] * q[QY];

    // roll (x-axis rotation)
    float t0 = +2.0f * (q[QW] * q[QX] + q[QY] * q[QZ]);
    float t1 = +1.0f - 2.0f * (q[QX] * q[QX] + ysqr);
    roll = atan2(t0, t1) * RAD2DEG;

    // pitch (y-axis rotation)
    float t2 = +2.0f * (q[QW] * q[QY] - q[QZ] * q[QX]);
    t2 = t2 > 1.0f ? 1.0f : t2;
    t2 = t2 < -1.0f ? -1.0f : t2;
    pitch = asin(t2) * RAD2DEG;

    // yaw (z-axis rotation)
    float t3 = +2.0f * (q[QW] * q[QZ] + q[QX] *q[QY]);
    float t4 = +1.0f - 2.0f * (ysqr + q[QZ] * q[QZ]);
    yaw = atan2(t3, t4) * RAD2DEG;
}


