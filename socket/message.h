#ifndef MESSAGE_H
#define MESSAGE_H

#include "endpoint.h"
#include <sys/ioctl.h>

class Message{
public:
    Message(){};
    Message(Endpoint end, char* data, int length, int rssi=-1);

    /*!
     * @brief Returns the IP address of the message sender
     * 
     * @return uint32_t 
     */
    uint32_t getAddressI(void) const;

    /*!
     * @brief Returns the IP address of the message sender
     * 
     * @return char array
     */
    char *getAddressC(void);

    /*!
     * @brief Get the Length of the message
     * 
     * @return int 
     */
    int getLength(void) const;

    /*!
     * @brief Get the Data of the message
     * 
     * @return char* 
     */
    char* getData(void);

    /*!
     * @brief Get the Port object
     * 
     * @return int 
     */
    int getPort(void) const;

    /*!
     * @brief Get the Endpoint object
     * 
     * @return Endpoint 
     */
    Endpoint& getEndpoint(void);

    /*!
     * @brief Get the received signal strength when this packet was received
     * 
     * @return int rssi (dBm), -1 if data does not exist
     */
    int getRssi(void) const;

private:
    Endpoint end;
    char* data;
    int length;
    int rssi;
};

#endif