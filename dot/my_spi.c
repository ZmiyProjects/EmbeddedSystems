//
// Created by zmiy on 09.11.2019.
//

#include "my_spi.h"

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>


/* Специальная структура для обмена данными по SPI шине */
struct spi_ioc_transfer xfer[2];

/* Инициализация SPI шины для обмена данными */
int spi_open(
        char *device,
        unsigned int speed
){
    int fd;
    unsigned char mode, lsb, bits;

    if ((fd = open(device, O_RDWR)) < 0){
        perror("spi_open() failed!");
        exit(1);
    }

    xfer[0].len = 0; /* Length of  command to write*/
    xfer[0].cs_change = 0; /* Keep CS activated */
    xfer[0].delay_usecs = 0, //delay in us
    xfer[0].speed_hz = speed, //speed
    xfer[0].bits_per_word = 8, // bites per word 8

    xfer[1].len = 0; /* Length of Data to read */
    xfer[1].cs_change = 0; /* Keep CS activated */
    xfer[1].delay_usecs = 0;
    xfer[1].speed_hz = speed;
    xfer[1].bits_per_word = 8;

    return fd;
}

/* Выполнение транзакции (прием-передача) по SPI шине */
void spi_rw(
        int fd,
        unsigned char *tx_buf,
        int tx_nbytes,
        unsigned char *rx_buf,
        int rx_nbytes
){
    int status;

    xfer[0].tx_buf = (int)tx_buf;
    xfer[0].len = tx_nbytes; /* Length of  command to write*/
    xfer[1].rx_buf = (int)rx_buf;
    xfer[1].len = rx_nbytes; /* Length of Data to read */
    status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);

    if (status < 0){
        perror("spi_rw() failed!");
        return;
    }
}

/* Запись N байт */
void spi_write(
        int fd,
        unsigned char *tx_buf,
        int tx_nbytes
){
    int status;

    xfer[0].tx_buf = (int)tx_buf;
    xfer[0].len = tx_nbytes; /* Length of  command to write*/

    status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);

    if (status < 0){
        perror("spi_write() failed!");
        return;
    }
}

/* Функция, завершающая работу с SPI шиной */
void spi_close(
        int fd
){
    close(fd);
}