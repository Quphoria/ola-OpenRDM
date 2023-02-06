#include <vector>
#include <string>

#include "ola/rdm/RDMCommand.h"
#include "ola/rdm/RDMCommandSerializer.h"
#include "plugins/openrdm/OpenRDMThread.h"

namespace ola {
namespace plugin {
namespace openrdm {

using ola::rdm::UID;
using ola::rdm::UIDSet;
using ola::rdm::RDMReply;

OpenRDMThread::OpenRDMThread(OpenRDMWidget *widget,
                      unsigned int dmx_refresh_ms,
                      bool rdm_enabled)
    :   m_widget(widget),
        m_dmx_refresh_ms(dmx_refresh_ms),
        m_rdm_enabled(rdm_enabled) {
    //
}

OpenRDMThread::~OpenRDMThread() {
    if (threads_running) Stop();
}

void OpenRDMThread::Start() {
    dmx_thread = std::thread(dmx_thread,
                             m_widget,
                             m_dmx_refresh_ms,
                             dmx_sema,
                             dmx_mutex,
                             dmx_data,
                             &exit_flag);
    if (m_rdm_enabled) {
        rdm_thread = std::thread(rdm_thread,
                                 m_widget,
                                 rdm_sema,
                                 rdm_mutex,
                                 rdm_data,
                                 tod_mutex,
                                 tod,
                                 &exit_flag);
    }
    threads_running = true;
}

void OpenRDMThread::Stop() {
    exit_flag = true;
    dmx_thread.join();
    rdm_thread.join();
    threads_running = false;
}

bool OpenRDMThread::WriteDMX(const ola::DmxBuffer &buffer) {
    dmx_mutex.lock();
    buffer.Get(dmx_data->data.begin(), &dmx_data->length);
    dmx_data->changed = true;
    dmx_mutex.unlock();

    dmx_sema->notify();

    buffer.Reset();
}

void OpenRDMThread::SendRDMRequest(ola::rdm::RDMRequest *request,
                                   ola::rdm::RDMCallback *callback) {
    if (!m_rdm_enabled) {
        RunRDMCallback(callback, ola::rdm::RDM_FAILED_TO_SEND);
        return;
    }
    if (request->CommandClass() == RDMCommand::DISCOVER_COMMAND) {
        RunRDMCallback(on_complete, ola::rdm::RDM_PLUGIN_DISCOVERY_NOT_SUPPORTED);
        return;
    }
    
    tod_mutex.lock();
    if (!tod->Contains(request->DestinationUID())) {
        RunRDMCallback(callback, ola::rdm::RDM_UNKNOWN_UID);
        return;
    }
    tod_mutex.unlock();

    RDMMessage msg;
    unsigned int length;
    msg.type = RDM_DATA;
    msg.request = request;
    msg.rdm_callback = callback;
    
    bool ok = true;

    rdm_mutex.lock();
    if (rdm_data->size() < RDM_QUEUE_MAX_LENGTH) {
        rdm_data->push(msg);
    } else {
        ok = false;
    }
    rdm_mutex.unlock();

    if (ok) {
        rdm_sema->notify();
    } else {
        RunRDMCallback(callback, ola::rdm::RDM_FAILED_TO_SEND);
    }
}

void OpenRDMThread::RunFullDiscovery(ola::rdm::RDMDiscoveryCallback *callback) {
    RDMMessage msg;
    msg.length = RDM_FULL_DISCOVERY;
    msg.disc_callback = callback;

    bool ok = true;

    rdm_mutex.lock();
    if (rdm_data->size() < RDM_QUEUE_MAX_LENGTH) {
        rdm_data->push(msg);
    } else {
        ok = false;
    }
    rdm_mutex.unlock();

    if (ok) {
        rdm_sema->notify();
    } else {
        tod_mutex.lock();
        UIDSet uid_set = UIDSet(tod);
        tod_mutex.unlock();
        callback->Run(uid_set);
    }
}

void OpenRDMThread::RunIncrementalDiscovery(ola::rdm::RDMDiscoveryCallback *callback) {
    RDMMessage msg;
    msg.length = RDM_INCREMENTAL_DISCOVERY;
    msg.disc_callback = callback;

    bool ok = true;

    rdm_mutex.lock();
    if (rdm_data->size() < RDM_QUEUE_MAX_LENGTH) {
        rdm_data->push(msg);
    } else {
        ok = false;
    }
    rdm_mutex.unlock();

    if (ok) {
        rdm_sema->notify();
    } else {
        tod_mutex.lock();
        UIDSet uid_set = UIDSet(tod);
        tod_mutex.unlock();
        callback->Run(uid_set);
    }
}

void dmx_thread(OpenRDMWidget *widget,
                unsigned int dmx_refresh_ms,
                std::shared_ptr<counting_semaphore<SEMA_MAX>> dmx_sema,
                std::mutex dmx_mutex,
                std::shared_ptr<DMXMessage> dmx_data,
                bool *exit_flag) {
    if (!widget->isInitialized()) return;

    auto dmx_timeout = std::chrono::milliseconds(dmx_refresh_ms);
    
    int length;
    uint8_t data[DMX_MAX_LENGTH];
    bool dmx_changed = false;
    auto t_last = std::chrono::high_resolution_clock::now();

    while (!(*exit_flag)) {
        bool sema_acquired = dmx_sema->wait_for(dmx_timeout);
        if (sema_acquired) {
            dmx_mutex.lock();
            dmx_changed = dmx_data->changed;
            if (dmx_changed) {
                length = dmx_data->length;
                std::copy_n(dmx_data->data.data(), length, data);
                dmx_data->changed = false;
            }
            dmx_mutex.unlock();

            if (dmx_changed) {
                widget->writeDMX(data, length);
                t_last = std::chrono::high_resolution_clock::now();
            }
        }
        
        auto t_now = std::chrono::high_resolution_clock::now();
        double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_now-t_last).count();
        if (!sema_acquired || elapsed_time_ms > dmx_refresh_ms) {
            // Timed out, DMX refresh
            dmx_mutex.lock();
            length = dmx_data->length;
            std::copy_n(dmx_data->data.data(), length, data);
            dmx_mutex.unlock();

            widget->writeDMX(data, length);
            t_last = std::chrono::high_resolution_clock::now();
        }
    }
}

void rdm_thread(OpenRDMWidget *widget,
                std::shared_ptr<counting_semaphore<SEMA_MAX>> rdm_sema,
                std::mutex rdm_mutex,
                std::shared_ptr<std::queue<RDMMessage>> rdm_data,
                std::mutex tod_mutex,
                std::shared_ptr<std::set<UID>> tod,
                bool *exit_flag) {
    // Not too fussed about RDM working just yet
    // Get DMX working first
    return;
    if (!widget->isInitialized()) return;

    auto rdm_timeout = std::chrono::milliseconds(RDM_SEMA_TIMEOUT_MS);

    while (!(*exit_flag)) {
        bool sema_acquired = sema->wait_for(rdm_timeout);
        if (sema_acquired) {
            rdm_mutex.lock();
            // Handle RDM messages 1 message at a time so we don't halt the dmx too much
            if (!rdm_data->empty()) {
                auto msg = rdm_data->front();
                rdm_data->pop();

                if (msg.type == RDM_DATA) {
                    uint8_t data[512];
                    unsigned int actual_len;
                    bool ok = RDMCommandSerializer::Pack(*(msg.request), data, &actual_len);

                    if (ok) {
                        // Check SUB START CODE (in case new RDM version has different packet structure)
                        if (actual_len > 2 && data[0] == RDM_SUB_START_CODE) {
                            actual_len = std::min(actual_len, 1+data[1]);
                        }
                        auto resp = widget->writeRDM(data, actual_len);
                        if (resp.first > 1 && resp.second[0] == RDM_START_CODE) {
                            std::auto_ptr<RDMReply> reply(RDMReply::FromFrame(
                                rdm::RDMFrame(resp.second, resp.first)
                            ));
                            msg.rdm_callback->Run(reply.get());
                        } else if (request->DestinationUID().IsBroadcast()) {
                            RDMReply reply(ola::rdm::RDM_WAS_BROADCAST);
                            msg.rdm_callback->Run(&reply);
                        } else {
                            RDMReply reply(ola::rdm::RDM_TIMEOUT);
                            msg.rdm_callback->Run(&reply);
                        }
                    } else {
                        RunRDMCallback(msg.disc_callback, ola::rdm::RDM_FAILED_TO_SEND);
                    }
                } else if (msg.length == RDM_FULL_DISCOVERY) {
                    auto new_tod = widget->fullRDMDiscovery();
                    tod_mutex.lock();
                    *tod = new_tod;
                    tod_mutex.unlock();
                    msg.disc_callback->Run(new_tod);
                } else if (msg.length == RDM_INCREMENTAL_DISCOVERY) {
                    auto tod_changes = widget->incrementalRDMDiscovery();
                    auto added = tod_changes.first;
                    auto removed = tod_changes.second;
                    tod_mutex.lock();
                    *tod = tod->Union(added).SetDifference(removed);
                    UIDSet uid_set = UIDSet(tod);
                    tod_mutex.unlock();
                    msg.disc_callback->Run(uid_set);
                }
            }
            rdm_mutex.unlock();
        }
    }
}

}  // namespace openrdm
}  // namespace plugin
}  // namespace ola