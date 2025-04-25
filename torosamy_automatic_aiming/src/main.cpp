#include "pod/manager/PacketManager.h"
#include "pod/manager/ModuleManager.h"
#include "pod/manager/SerialPortManager.h"
#include "pod/manager/CameraManager.h"
#include "torosamy_automatic_aiming/ArmorModule.h"
#include "torosamy_automatic_aiming/HeartJumpModule.h"
#include "torosamy_automatic_aiming/FunctionController.h"
int main() {
    using namespace Torosamy;
    CameraManager::getInstance()->openCameras();
    CameraManager::getInstance()->startUpdateCameras();

    PacketManager::getInstance()
        ->addReceivePacket(std::make_shared<ReceiveDataPacket>(0))
        ->addSendPacket(std::make_shared<SendDataPacket>(0));

    SerialPortManager::getInstance()->detachTasks();

    ModuleManager::getInstance()
        ->addModules(ArmorModule::makeModules())
        ->addModule(HeartJumpModule::makeModule())
        ->detachTasks();

        
    int option = 0;


    while(true) {
        std::cin>>option;

        switch (option)
        {
        case 1:
            FunctionController::disableShowSrc();
            break;
        case 2:
            FunctionController::enableShowSrc();
            break;
        case 3:
            SerialPortManager::getInstance()->reloadSerialPorts();
            SerialPortManager::getInstance()->detachTasks();
            break;
        case 4:
            CameraManager::getInstance()->startUpdateCameras();
            break;
        case 5:
            CameraManager::getInstance()->stopUpdateCameras();
            break;
        case 6:
            CameraManager::getInstance()->openCameras();
            CameraManager::getInstance()->startUpdateCameras();
            break;    
        case 7:
            CameraManager::getInstance()->stopCameras();
            break;  
        default:
            break;
        }

    }

    return 0;
}
