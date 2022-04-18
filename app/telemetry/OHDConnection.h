#ifndef OHDMAVLINKCONNECTION_H
#define OHDMAVLINKCONNECTION_H

#include <QObject>
#include <QtQuick>
//#include <QTimer>


#include <openhd/mavlink.h>

#include "../util/util.h"

/**
 * @brief This is the one and only (mavlink telemetry) connection of QOpenHD to OpenHD
 * (More specific, the OpenHD Ground Station - but since the Ground station forwards messages to the air pi,
 * one can indirectly also reach the air pi via this callback)
 * Its functionalities are simple:
 * 1) sending mavlink messages to OpenHD
 * 2) receiving mavlink messages from OpenHD
 *
 * The received mavlink messages can come from OpenHD itself (custom) or can be telemetry data from the Drone Flight Controller
 * connected to the OpenHD air unit.
 *
 * If we go for a single TCP or 2 udp connections (1 for send, one for receive) is not clear yet.
 *
 * The pulic methods of this class won't change regardeless.
 *
 * If the connection to OpenHD is lost, this class should try and reconnect in intervalls until the connection has been re-established.
 *
 * NOTE: Since QOpenHD does the same, this one sends out mavlink heartbeat messages in reqular intervalls
 */

class OHDConnection : public QObject
{
public:
    OHDConnection(QObject *parent = nullptr,bool useTcp=true);
    /**
     * Once started, this class continiosly tries to establish a connection, re-establishing the connection when the connection is lost.
     */
    void start();
    /**
     * Stop the re-establishing connection functionality, and close the connection if established. Once this call returns, it is quaranteed
     * that no more messages are fed to the callback.
     */
    void stop();
    /**
     * @brief MAV_MSG_CALLBACK Callback that can be registered and is called every time a new message from the OpenHD
     * Ground station has been parsed
     */
    typedef std::function<void(mavlink_message_t msg)> MAV_MSG_CALLBACK;
    void registerNewMessageCalback(MAV_MSG_CALLBACK cb);
    /**
     * @brief sendMessage send a message to the OHD Ground station. If no connection has been established (yet), this should return immediately.
     * @param msg the message to send to the OHD Ground Station
     */
    void sendMessage(mavlink_message_t msg);
    // I use the same SYS ID as QGroundControll here - we may need to change this
    static constexpr auto QOHD_MAVLINK_SYS_ID=255;
private:
    // I am still not sure if we shall use TCP or UDP for the connection between QOpenHD
    // and OpenHD.
    const bool USE_TCP;
    static constexpr auto QOPENHD_GROUND_CLIENT_TCP_ADDRESS="127.0.0.1";
    static constexpr auto QOPENHD_GROUND_CLIENT_TCP_PORT=1234;
    static constexpr auto QOPENHD_GROUND_CLIENT_UDP_ADDRESS="127.0.0.1";
    static constexpr auto QOPENHD_GROUND_CLIENT_UDP_PORT_IN=14550;
    static constexpr auto QOPENHD_GROUND_CLIENT_UDP_PORT_OUT=14551;
    /**
     * @brief Re-Connect if connection has been lost
     * else, do nothing
     */
    void reconnectIfDisconnected();
    void parseNewData(const uint8_t* data, int data_len);
    MAV_MSG_CALLBACK callback=nullptr;
    mavlink_status_t receiveMavlinkStatus{};
    void sendData(const uint8_t* data,int data_len);
private:
    QTimer* reconnectTimer= nullptr;
    static constexpr auto HARTBEAT_INTERVALL_SECONDS=1;
    QTimer* heartbeatTimer=nullptr;
    QTcpSocket *tcpClientSock = nullptr;
    QUdpSocket *udpSocket=nullptr;
private slots:
    // called by QT tcp socket
    void tcpReadyRead();
    void tcpConnected();
    void tcpDisconnected();
    // called by QT udp socket
    void udpReadyRead();
    // called by heartbeat timer
    void onHeartbeat();
};

#endif // OHDMAVLINKCONNECTION_H