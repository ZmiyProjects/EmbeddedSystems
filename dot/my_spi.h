//
// Created by zmiy on 09.11.2019.
//

#ifndef SPI_MAIN_MY_SPI_H
#define SPI_MAIN_MY_SPI_H

int spi_open(
        char *device,
        unsigned int speed
);

void spi_rw(
        int fd,
        unsigned char *tx_buf,
        int tx_nbytes,
        unsigned char *rx_buf,
        int rx_nbytes
);

void spi_write(
        int fd,
        unsigned char *buf,
        int nbytes
);

void spi_close(
        int fd
);

#endif //SPI_MAIN_MY_SPI_H
