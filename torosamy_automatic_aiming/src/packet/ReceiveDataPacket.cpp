#include "torosamy_automatic_aiming/packet/ReceiveDataPacket.h"
#include <iostream>
ReceiveDataPacket::ReceiveDataPacket(const int& id):
    ReceiveDataPacketInterface(id){
    this->mSize = 11;
    initData();
}


void ReceiveDataPacket::initData() {
    this->pitch.f = 0;
    this->yaw.f = 0;
    this->heart_jump_num.s = 0;
    this->motion_enable.b = false;
}

int ReceiveDataPacket::readData(const unsigned char* const dataArr,const int& startIndex) {
    int index = 0;
    this->pitch.c[0] = dataArr[startIndex + index++];
    this->pitch.c[1] = dataArr[startIndex + index++];
    this->pitch.c[2] = dataArr[startIndex + index++];
    this->pitch.c[3] = dataArr[startIndex + index++];

    this->yaw.c[0] = dataArr[startIndex + index++];
    this->yaw.c[1] = dataArr[startIndex + index++];
    this->yaw.c[2] = dataArr[startIndex + index++];
    this->yaw.c[3] = dataArr[startIndex + index++];


    this->heart_jump_num.c[0] = dataArr[startIndex + index++];
    this->heart_jump_num.c[1] = dataArr[startIndex + index++];

    this->motion_enable.c[0] = dataArr[startIndex + index++];
    return mSize;
}
