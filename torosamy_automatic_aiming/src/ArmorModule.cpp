#include "torosamy_automatic_aiming/ArmorModule.h"

#include <pod/manager/ModuleManager.h>

#include "torosamy_automatic_aiming/armor/ArmorPnpResult.h"

#include "utils/MessageUtils.h"
#include "pod/manager/CameraManager.h"
#include "pod/manager/PacketManager.h"


std::vector<int> ArmorModule::mIds;

void ArmorModule::run() {
    using namespace Torosamy;
    const std::string id = std::to_string(mId);

    const int maxTryTimes = 5;
    for(int i = 0;i < maxTryTimes;i++) {
        if(mCamera->opened()) break;

        if(i == maxTryTimes - 1) {
            std::cerr<<"the module: "<<mId<<" can not get opened camera, now break thread :("<<std::endl;
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    while (mRunning) {
        try{
            const TIME startTime = MessageUtils::getTimePoint();

            if (!mCamera->updateSrc()) {
                mSendDataPacket->initData();

                for(int i = 0;i < maxTryTimes;i++) {
                    if(mCamera->releaseCamera()) break;
                    if(i == maxTryTimes - 1) {
                        std::cerr<<"fail to release the module: "<<mId<<" camera, now break thread :("<<std::endl;
                        return;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                if (!mCamera->openCamera()) {
                    if(!mSrc.empty()) cv::destroyWindow(id);
                    std::cerr<<"fail to restart camera and has been try to 5 times, now break thread :("<<std::endl;
                    break;
                }
            }

            const bool doSuccessful = doOnce();


            if(mShowSrc) {
                if(!mSrc.empty()) drawParams(startTime);
                if(!mSrc.empty()) imshow(id, mSrc);
            }else {
                cv::Mat img(cv::Size(50, 20), CV_8UC3, cv::Scalar(0, 0, 0));
                Torosamy::MessageUtils::putText(img, std::to_string(Torosamy::MessageUtils::getFpsByTimePoint(startTime)), cv::Point2f(0, 20));
                if(!mSrc.empty()) imshow(id, img);
            }
            
    		cv::waitKey(mCamera->getTimeOff());

        }catch(const std::runtime_error& e) {
            std::cerr<<e.what()<<std::endl;
        }
    }
}

bool ArmorModule::doOnce() {
    if (!mCamera->cloneSrc(mSrc)) {
        handlerLostTrack();
        return false;
    }    
    if(!findArmor())  {
        handlerLostTrack();
        return false;
    }
    if(!mArmorManager.selectTarget()) {
    	handlerLostTrack();
    	return false;
	}

    const ArmorPnpResult& targetArmorPnpResult = getPnpResult();


	mLostFrameHandler.trackTarget();
    mArmorManager.drawTarget(mSrc);

    mSendDataPacket->yaw.f = mFireControlHandler.getYaw(targetArmorPnpResult);
    mSendDataPacket->pitch.f = mFireControlHandler.getPitch(targetArmorPnpResult);
    mSendDataPacket->is_find_target.b = true;
    mSendDataPacket->distance.f = targetArmorPnpResult.getDistance();
    mSendDataPacket->start_fire.b = shouldFire();

    return true;
}


bool ArmorModule::findArmor() {

	if(!mLightManager.findLights(mSrc, Torosamy::targetColor)) {
    	handlerLostTrack();
    	return false;
	}
    mLightManager.drawLights(mSrc);
    
    if(!mArmorManager.findArmors(mLightManager.getLights())) {
        mArmorManager.resetTargetArmorType();
        handlerLostTrack();
    	return false;
    }
    mArmorManager.drawArmors(mSrc);
    
    if (!mArmorManager.findArmors(mSrc)) {
        mArmorManager.resetTargetArmorType();
        handlerLostTrack();
        return false;
    }

    return true;
}




ArmorPnpResult ArmorModule::getPnpResult() {
    const Armor& targetArmor = mArmorManager.getTargetArmor();

    // if (targetArmor.getArmorType() != ArmorType::OUTPOST) {
    //     // std::cout << static_cast<int>(targetArmor.getArmorType()) <<std::endl;
    // }

    
    const ArmorPnpResult& armor_pnp_result = ArmorPnpResult(
        mCamera->getCameraMatrix(),
        mCamera->getDistCoeffs(),
        mReceiveDataPacket->pitch.f,
        mReceiveDataPacket->yaw.f,
        targetArmor
    );

    if (!mPredictHandler.enabled()) return armor_pnp_result;

    if (mPredictHandler.getMode() == PredictMode::POINT) {
        const ArmorType& armorType = targetArmor.getArmorType();

        const cv::Point2f predictPoint = armorType == ArmorType::OUTPOST ?
        mPredictHandler.mPointPredicter.getPredictPoint(mFireControlHandler.getFilghtTime(), targetArmor,mReceiveDataPacket->yaw.f,mReceiveDataPacket->pitch.f) :
        mPredictHandler.mPointPredicter.getPredictPoint(mFireControlHandler.getFilghtTime(), targetArmor);

        //TODO
        mSendDataPacket->is_turn_right.b = predictPoint.x > targetArmor.getCenter().x;

        if (mPredictHandler.isUpdateResult()) {
            return ArmorPnpResult(
                mCamera->getCameraMatrix(),
                mCamera->getDistCoeffs(),
                mReceiveDataPacket->pitch.f,
                mReceiveDataPacket->yaw.f,
                targetArmor.generatePredictArmor(predictPoint)
            );
        }
    }





    if (mPredictHandler.getMode() == PredictMode::MOTION) {
        if (mLostFrameHandler.isLostFrame()) {
            mPredictHandler.mMotionPredicter.init(armor_pnp_result);
        }

        std::vector<ArmorPnpResult> armorPnpResults;
        for (const auto& armor : mArmorManager.getArmors()) {
            armorPnpResults.emplace_back(
                ArmorPnpResult(
                    mCamera->getCameraMatrix(),
                    mCamera->getDistCoeffs(),
                    mReceiveDataPacket->pitch.f,
                    mReceiveDataPacket->yaw.f,
                    armor
                )
            );
        }
        if (mPredictHandler.mMotionPredicter.update(armorPnpResults)) {
            
            //TODO
            mSendDataPacket->is_turn_right.b = mPredictHandler.mMotionPredicter.isTurnRight();

            if (mPredictHandler.isUpdateResult()) return armor_pnp_result;


            const Eigen::Vector3d& tvec = mPredictHandler.mMotionPredicter.generatePredictTvec(
                mFireControlHandler.getFilghtTime(),
                mReceiveDataPacket->pitch.f,
                mReceiveDataPacket->yaw.f
            );
            
            return ArmorPnpResult(tvec, targetArmor);
        }

        mSendDataPacket->is_turn_right.b = false;

    }

    return armor_pnp_result;
}


bool ArmorModule::handlerLostTrack() {
  	mLostFrameHandler.lostTrack();
    if (mLostFrameHandler.isLostFrame()) {
        mSendDataPacket->initData();
        return true;
    }
    return false;
}



std::vector<std::shared_ptr<Torosamy::TorosamyModule>> ArmorModule::makeModules() {
    std::vector<std::shared_ptr<Torosamy::TorosamyModule>> result;
    const std::string configName = Torosamy::targetColor == Torosamy::ColorType::RED ? "target_red_config.yml" : "target_blue_config.yml";
    
    const YAML::Node fileReader = YAML::LoadFile(Torosamy::TorosamyModule::getConfigLocation("armor")+configName);

    std::vector<int> ids;
    for(const auto& idNode : fileReader["LoadIds"]) {
        ids.push_back(idNode.as<int>());
    }


    if(ids.empty()) {
        std::cout<<"no id for armor found"<<std::endl;
        return result;
    }

    for(const int& id : ids) {
        if (Torosamy::ModuleManager::getInstance()->isRepeat(id)) continue;
        mIds.emplace_back(id);
        result.emplace_back(std::make_shared<ArmorModule>(id));
    }

    return result;
}

ArmorModule::ArmorModule(const int& id) : 
    TorosamyModule(id),
    mLightManager(getConfigNode(id)["Light"]),
    mArmorManager(getConfigNode(id)["Armor"]),
    mFireControlHandler(getConfigNode(id)["FireControl"]),
    mPredictHandler(getConfigNode(id)["Predict"]){
	std::string stringId = std::to_string(id);
    const YAML::Node fileReader = getConfigNode(id);

    const std::string configName = Torosamy::targetColor == Torosamy::ColorType::RED ? "target_red_config.yml" : "target_blue_config.yml";

    std::cout <<"module id: "<<id<< ", read config: "<<configName <<std::endl;

    mShowSrc = fileReader["ShowSrc"].as<bool>();
    mCamera = Torosamy::CameraManager::getInstance()->getCameraById<Torosamy::MindCamera>(fileReader["CameraID"].as<int>());
    mSendDataPacket = Torosamy::PacketManager::getInstance()->getSendPacketById<SendDataPacket>(fileReader["SendId"].as<int>());
    mReceiveDataPacket = Torosamy::PacketManager::getInstance()->getReceivePacketById<ReceiveDataPacket>(fileReader["ReceiveID"].as<int>());
}


bool ArmorModule::shouldFire() const{
    if (!mSendDataPacket->is_find_target.b) return false;

	const float yaw = fabs(mSendDataPacket->yaw.f);
    const float pitch = fabs(mSendDataPacket->pitch.f);
    const float distance = mSendDataPacket->distance.f;

    return mFireControlHandler.shouldFire(distance, yaw, pitch);
}


void ArmorModule::drawParams(const Torosamy::TIME& startTime) {
    using namespace Torosamy;

    std::string params[10] = {
        "pitch:" + std::to_string(mSendDataPacket->pitch.f),
        "yaw:" + std::to_string(mSendDataPacket->yaw.f),
        "distance:" + std::to_string(mSendDataPacket->distance.f),
        "find_target:" + std::to_string(mSendDataPacket->is_find_target.b),
        "start_fire:" + std::to_string(mSendDataPacket->start_fire.b),
        "gain_yaw:" + std::to_string(mReceiveDataPacket->yaw.f),
        "gain_pitch:" + std::to_string(mReceiveDataPacket->pitch.f),
        "turn_right:" + std::to_string(mSendDataPacket->is_turn_right.b),
        "target_type:" + std::to_string(static_cast<int>(mArmorManager.getTargetArmor().getArmorType())),
        "fps:" + std::to_string(MessageUtils::getFpsByTimePoint(startTime))
    };


    for(int i = 0;i<10;i++) {
        MessageUtils::putText(mSrc, params[i], cv::Point(0, (i + 1) * 20));
    }


    // 纵坐标
    cv::line(mSrc, cv::Point2f(mSrc.size().width / 2, 0), cv::Point2f(mSrc.size().width / 2, mSrc.size().height), cv::Scalar(208, 224, 64), 1, 8);
    // 横坐标
    cv::line(mSrc, cv::Point2f(0, mSrc.size().height / 2), cv::Point2f(mSrc.size().width, mSrc.size().height / 2), cv::Scalar(208, 224, 64), 1, 8);
    // 预估击打横坐标
    cv::line(mSrc, cv::Point2f(mSrc.size().width / 2 - mSrc.size().width / 6, mSrc.size().height / 2 + 40), cv::Point2f(mSrc.size().width / 2 + mSrc.size().width / 6, mSrc.size().height / 2 + 40), cv::Scalar(215, 235, 250), 1, 8);
    cv::line(mSrc, cv::Point2f(mSrc.size().width / 2 - mSrc.size().width / 8, mSrc.size().height / 2 + 80), cv::Point2f(mSrc.size().width / 2 + mSrc.size().width / 8, mSrc.size().height / 2 + 80), cv::Scalar(215, 235, 250), 1, 8);

}



std::vector<int> ArmorModule::getIds() {
    return mIds;
}

YAML::Node ArmorModule::getConfigNode(const int& id) {
    const std::string configName = Torosamy::targetColor == Torosamy::ColorType::RED ? "target_red_config.yml" : "target_blue_config.yml";

    // std::cout <<"module id: "<<id<< ", read config: target_red_config.yml"<<std::endl;

    return YAML::LoadFile(getConfigLocation("armor")+ configName)[std::to_string(id)];
}

void ArmorModule::enableShow() {
    mShowSrc = true;
}
void ArmorModule::disableShow() {
    mShowSrc = false;
}

