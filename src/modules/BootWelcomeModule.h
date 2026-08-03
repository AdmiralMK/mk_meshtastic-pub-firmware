#pragma once
#include "SinglePortModule.h"
#include "concurrency/OSThread.h"
#include "configuration.h"

class BootWelcomeModule : public SinglePortModule, private concurrency::OSThread
{
  public:
    BootWelcomeModule();

  protected:
    virtual int32_t runOnce() override;
    virtual ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override { return ProcessMessage::CONTINUE; }

  private:
    bool sent = false;
};

extern BootWelcomeModule *bootWelcomeModule;