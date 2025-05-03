#include "torosamy_automatic_aiming/packet/SendDataPacket.h"
#include "opencv2/opencv.hpp"
#include <iostream>


SendDataPacket::SendDataPacket(const int& id):
    SendDataPacketInterface(id) {
    this->mSize = 12;
    initData();
}

void SendDataPacket::initData() {
    this->pitch.f = 0;
    this->yaw.f = 0;
    this->heart_jump_num.s = 0;
    this->is_find_target.b = false;
    this->start_fire.b = false;
    this->distance.f = 0;
}

int SendDataPacket::writeData(unsigned char* dataArr,const int& startIndex) {
    int index = 0;
    //if(this->pitch.f>=15||pitch.f<=-15) this->pitch.f=0;
    dataArr[startIndex + index++] = this->pitch.c[0];
    dataArr[startIndex + index++] = this->pitch.c[1];
    dataArr[startIndex + index++] = this->pitch.c[2];
    dataArr[startIndex + index++] = this->pitch.c[3];


    //if(this->yaw.f>=15||yaw.f<=-15) this->yaw.f=0;
    dataArr[startIndex + index++] = this->yaw.c[0];
    dataArr[startIndex + index++] = this->yaw.c[1];
    dataArr[startIndex + index++] = this->yaw.c[2];
    dataArr[startIndex + index++] = this->yaw.c[3];


    this->heart_jump_num.s = (this->heart_jump_num.s + 1) % 10;
    dataArr[startIndex + index++] = this->heart_jump_num.c[0];
    dataArr[startIndex + index++] = this->heart_jump_num.c[1];


    dataArr[startIndex + index++] = this->is_find_target.c[0];
    dataArr[startIndex + index++] = this->start_fire.c[0];

    return mSize;
}


