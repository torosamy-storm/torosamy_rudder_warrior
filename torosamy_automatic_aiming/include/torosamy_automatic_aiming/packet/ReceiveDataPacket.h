#ifndef RECEIVEDATAPACKET_H
#define RECEIVEDATAPACKET_H

#include "packet/ReceiveDataPacketInterface.h"
#include <memory>


class ReceiveDataPacket : public Torosamy::ReceiveDataPacketInterface {
    using short2uchar = Torosamy::short2uchar;
    using float2uchar = Torosamy::float2uchar;
    using bool2uchar = Torosamy::bool2uchar;
public:
    ReceiveDataPacket(const int& id);
    int readData(const unsigned char* dataArr,const int& startIndex) override;
    void initData() override;

    bool2uchar motionEnable;
    short2uchar heartJumpNum;
    float2uchar pitch;
    float2uchar yaw;
};



#endif
