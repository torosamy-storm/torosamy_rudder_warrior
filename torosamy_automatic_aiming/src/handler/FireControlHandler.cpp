//
// Created by torosamy on 25-3-27.
//

#include "torosamy_automatic_aiming/handler/FireControlHandler.h"
#include <opencv2/opencv.hpp>
const float FireControlHandler::K = 0.020f;
const float FireControlHandler::GRAVITY = 9.78f;
const float FireControlHandler::V = 28.0f;


float FireControlHandler::getYaw(const ArmorPnpResult& armor) const{
    const double x = armor.getPose().x() * 1000;
    const double z = armor.getPose().z() * 1000;

    const float result = -atan(x / z) * 180 / CV_PI;

    const bool isArmorMode = armor.getArmorType() != ArmorType::OUTPOST;
    const std::vector<float>& offests = isArmorMode ? mYawOffestsArmor : mYawOffestsOutpost;
    const float distance = armor.getDistance();

    if (0 < distance && distance <= 0.85) return result + offests.at(0);
    if (0.85 < distance && distance <= 1.35) return result + offests.at(1);
    if (1.35 < distance && distance <= 1.85) return result + offests.at(2);
    if (1.85 < distance && distance <= 2.35) return result + offests.at(3);
    if (2.35 < distance && distance <= 2.85) return result + offests.at(4);
    if (2.85 < distance && distance <= 3.35) return result + offests.at(5);
    if (3.35 < distance && distance <= 3.85) return result + offests.at(6);
    if (3.85 < distance && distance <= 4.35) return result + offests.at(7);
    if (4.35 < distance && distance <= 4.85) return result + offests.at(8);
    if (4.85 < distance && distance <= 5.35) return result + offests.at(9);
    if (5.35 < distance && distance <= 5.85) return result + offests.at(10);
    if (5.85 < distance && distance <= 6.35) return result + offests.at(11);
    if (6.35 < distance && distance <= 6.85) return result + offests.at(12);
    if (6.85 < distance && distance <= 7.35) return result + offests.at(13);
    if (7.35 < distance && distance <= 7.85) return result + offests.at(14);
    return result;
}

float FireControlHandler::getPitch(const ArmorPnpResult& armor){
    const bool isArmorMode = armor.getArmorType() != ArmorType::OUTPOST;
	const float bulletPitch = isArmorMode ? mBulletPitchOutpost : mBulletPitchArmor;
    const double y = armor.getPose().y() * 1000;
    const double z = armor.getPose().z() * 1000;


    const float result = -computePitch(z / 1000, -(y-bulletPitch) / 1000) * 180 / CV_PI;
    // sendPitch = sendPitch * 180 / CV_PI;

	const std::vector<float>& offests = isArmorMode ? mPitchOffestsArmor : mPitchOffestsOutpost;
    const float distance = armor.getDistance();

    if (0 < distance && distance <= 0.85) return result + offests.at(0);
    if (0.85 < distance && distance <= 1.35) return result + offests.at(1);
    if (1.35 < distance && distance <= 1.85) return result + offests.at(2);
    if (1.85 < distance && distance <= 2.35) return result + offests.at(3);
    if (2.35 < distance && distance <= 2.85) return result + offests.at(4);
    if (2.85 < distance && distance <= 3.35) return result + offests.at(5);
    if (3.35 < distance && distance <= 3.85) return result + offests.at(6);
    if (3.85 < distance && distance <= 4.35) return result + offests.at(7);
    if (4.35 < distance && distance <= 4.85) return result + offests.at(8);
    if (4.85 < distance && distance <= 5.35) return result + offests.at(9);
    if (5.35 < distance && distance <= 5.85) return result + offests.at(10);
    if (5.85 < distance && distance <= 6.35) return result + offests.at(11);
    if (6.35 < distance && distance <= 6.85) return result + offests.at(12);
    if (6.85 < distance && distance <= 7.35) return result + offests.at(13);
    if (7.35 < distance && distance <= 7.85) return result + offests.at(14);
    return result;
}


float FireControlHandler::computePitch(const float& depth, const float& height){
    //未计算y轴空气阻力
    //用tempHeight更新tempPitch 再用tempPitch更新dy 再用dy更新tempHeight
    float tempHeigh = height;
    float tempPitch = 0;
    float dy;


    for (int i = 0; i < 20; i++) {
        tempPitch = atan2(tempHeigh, depth);
        updateFilghtTime(depth, tempPitch);
        dy = height - bulletModel(tempPitch);
        tempHeigh += dy;

        if (fabsf(dy) < 0.001f) break;
    }
    return tempPitch;
}

float FireControlHandler::bulletModel(const float& pitch) const{
    return V * sin(pitch) * mFilghtTime - GRAVITY * mFilghtTime * mFilghtTime / 2;
}

void FireControlHandler::updateFilghtTime(const float& depth, const float& pitch) {
    mFilghtTime = (exp(K * depth) - 1) / (K * V * cos(pitch));
}

FireControlHandler::FireControlHandler(const YAML::Node& fileReader) :
    mBulletPitchArmor(fileReader["Armor"]["BulletPitch"].as<float>()),
    mBulletPitchOutpost(fileReader["Outpost"]["BulletPitch"].as<float>()),
    mFilghtTime(0){
    for(const auto& node : fileReader["Armor"]["OffestPitch"]) {
        mPitchOffestsArmor.push_back(node.as<float>());
    }
    for(const auto& node : fileReader["Armor"]["OffestYaw"]) {
        mYawOffestsArmor.push_back(node.as<float>());
    }

    for(const auto& node : fileReader["Outpost"]["OffestPitch"]) {
        mPitchOffestsOutpost.push_back(node.as<float>());
    }
    for(const auto& node : fileReader["Outpost"]["OffestYaw"]) {
        mYawOffestsOutpost.push_back(node.as<float>());
    }

}

float FireControlHandler::getFilghtTime() const{
    return this->mFilghtTime;
}