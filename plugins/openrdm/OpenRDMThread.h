
#ifndef PLUGINS_OPENRDM_OPENRDMTHREAD_H_
#define PLUGINS_OPENRDM_OPENRDMTHREAD_H_

#include <memory>
#include <thread>
#include <mutex>
#include <array>
#include <queue>

#include "ola/DmxBuffer.h"
#include "ola/rdm/UID.h"
#include "ola/rdm/UIDSet.h"
#include "olad/Port.h"

#include "plugins/openrdm/OpenRDMWidget.h"
#include "plugins/openrdm/rdm.h"
#include "plugins/openrdm/dmx.h"
#include "plugins/openrdm/Semaphore.h"

namespace ola {
namespace plugin {
namespace openrdm {

using ola::rdm::UID;
using ola::rdm::UIDSet;

static const unsigned int SEMA_MAX = 0xffff;
static const unsigned int RDM_SEMA_TIMEOUT_MS = 1000;
static const unsigned int RDM_QUEUE_MAX_LENGTH = 100;
static const unsigned int THREAD_REINIT_TIMEOUT_MS = 1000;

static const unsigned int RDM_DATA = 0;
static const unsigned int RDM_FULL_DISCOVERY = 1;
static const unsigned int RDM_INCREMENTAL_DISCOVERY = 2;

struct RDMMessage {
    unsigned int type;
    ola::rdm::RDMRequest *request;
    ola::rdm::RDMCallback *rdm_callback;
    ola::rdm::RDMDiscoveryCallback *disc_callback;
};

struct DMXMessage {
    bool changed;
    unsigned int length;
    std::array<uint8_t, DMX_MAX_LENGTH> data;
};

void dmx_thread_f(OpenRDMWidget *widget,
                unsigned int dmx_refresh_ms,
                std::shared_ptr<counting_semaphore<SEMA_MAX>> dmx_sema,
                std::shared_ptr<std::mutex> dmx_mutex,
                std::shared_ptr<DMXMessage> dmx_data,
                std::shared_ptr<bool> exit_flag);

void rdm_thread_f(OpenRDMWidget *widget,
                std::shared_ptr<counting_semaphore<SEMA_MAX>> rdm_sema,
                std::shared_ptr<std::mutex> rdm_mutex,
                std::shared_ptr<std::queue<RDMMessage>> rdm_data,
                std::shared_ptr<std::mutex> tod_mutex,
                std::shared_ptr<UIDSet> tod,
                std::shared_ptr<bool> exit_flag);

class OpenRDMThread {
    public:
        OpenRDMThread(OpenRDMWidget *widget,
                      unsigned int dmx_refresh_ms,
                      bool rdm_enabled);
        ~OpenRDMThread();

        void Start();
        void Stop();
        
        bool WriteDMX(const ola::DmxBuffer &buffer);
        void SendRDMRequest(ola::rdm::RDMRequest *request,
                            ola::rdm::RDMCallback *callback);
        void RunFullDiscovery(ola::rdm::RDMDiscoveryCallback *callback);
        void RunIncrementalDiscovery(ola::rdm::RDMDiscoveryCallback *callback);
    private:
        OpenRDMWidget *m_widget;
        unsigned int m_dmx_refresh_ms;
        bool m_rdm_enabled;
        bool threads_running = false;

        std::thread dmx_thread;
        std::thread rdm_thread;

        std::shared_ptr<counting_semaphore<SEMA_MAX>> dmx_sema =
            std::shared_ptr<counting_semaphore<SEMA_MAX>>(new counting_semaphore<SEMA_MAX>());
        std::shared_ptr<counting_semaphore<SEMA_MAX>> rdm_sema =
            std::shared_ptr<counting_semaphore<SEMA_MAX>>(new counting_semaphore<SEMA_MAX>()); 
        std::shared_ptr<std::mutex> dmx_mutex =
            std::shared_ptr<std::mutex>(new std::mutex());
        std::shared_ptr<std::mutex> rdm_mutex =
            std::shared_ptr<std::mutex>(new std::mutex());
        std::shared_ptr<DMXMessage> dmx_data =
            std::shared_ptr<DMXMessage>(new DMXMessage());
        std::shared_ptr<std::queue<RDMMessage>> rdm_data =
            std::shared_ptr<std::queue<RDMMessage>>(new std::queue<RDMMessage>());
        std::shared_ptr<bool> exit_flag =
            std::shared_ptr<bool>(new bool);

        std::shared_ptr<std::mutex> tod_mutex =
            std::shared_ptr<std::mutex>(new std::mutex());
        std::shared_ptr<UIDSet> tod =
            std::shared_ptr<UIDSet>(new UIDSet()); 
        
        DISALLOW_COPY_AND_ASSIGN(OpenRDMThread);
};

}  // namespace openrdm
}  // namespace plugin
}  // namespace ola

#endif // PLUGINS_OPENRDM_OPENRDMTHREAD_H_