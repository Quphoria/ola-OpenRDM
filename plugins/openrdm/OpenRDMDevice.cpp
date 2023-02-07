#include <string>
#include <memory>
#include "ola/Logging.h"
#include "ola/StringUtils.h"
#include "ola/rdm/UIDSet.h"
#include "plugins/openrdm/OpenRDMDevice.h"

namespace ola {
namespace plugin {
namespace openrdm {

using std::string;

bool verbose = true;
bool rdm_debug = false;

OpenRDMDevice::OpenRDMDevice(AbstractPlugin *owner,
                             const unsigned int &port_id,
                             const std::string &device_string,
                             const unsigned int &dmx_refresh_ms,
                             const bool &rdm_enabled)
    : Device(owner, "OpenRDM"),
      m_port_id(port_id),
      m_dev_str(device_string),
      m_dmx_refresh_ms(dmx_refresh_ms),
      m_rdm_enabled(rdm_enabled) {

  m_widget = new OpenRDMWidget(port_id, device_string, verbose, rdm_enabled, rdm_debug);
}

OpenRDMDevice::~OpenRDMDevice() {
  m_widget->deinit();
  delete m_widget;
}

bool OpenRDMDevice::StartHook() {
  AddPort(new OpenRDMOutputPort(this, m_port_id, m_widget, m_dev_str, m_dmx_refresh_ms, m_rdm_enabled));
  return true;
}

OpenRDMOutputPort::OpenRDMOutputPort(OpenRDMDevice *parent,
                                     unsigned int id,
                                     OpenRDMWidget *widget,
                                     const std::string &device_string,
                                     unsigned int dmx_refresh_ms,
                                     bool rdm_enabled)
  : BasicOutputPort(parent, id, rdm_enabled, rdm_enabled),
    m_widget(widget),
    m_port_id(id),
    m_thread(widget, dmx_refresh_ms, rdm_enabled),
    m_dev_str(device_string),
    m_dmx_refresh_ms(dmx_refresh_ms),
    m_rdm_enabled(rdm_enabled) {
  if (m_widget->init()) {
    OLA_DEBUG << "OpenRDM Port " << id << " (" << device_string << ") Initialized";
    m_thread.Start();
  } else {
    OLA_WARN << "OpenRDM Port " << id << " (" << device_string << ") Failed to Initialize";
  }
}

OpenRDMOutputPort::~OpenRDMOutputPort() {
  m_thread.Stop();
}

bool OpenRDMOutputPort::WriteDMX(const ola::DmxBuffer &buffer, uint8_t) {
  return m_thread.WriteDMX(buffer);
}

void OpenRDMOutputPort::SendRDMRequest(ola::rdm::RDMRequest *request,
                                       ola::rdm::RDMCallback *callback) {
  if (!m_rdm_enabled) {
    RunRDMCallback(callback, ola::rdm::RDM_FAILED_TO_SEND);
    return;
  }
  m_thread.SendRDMRequest(request, callback);
}

void OpenRDMOutputPort::RunFullDiscovery(ola::rdm::RDMDiscoveryCallback *callback) {
  if (!m_rdm_enabled) {
    UIDSet uid_set;
    callback->Run(uid_set);
    return;
  }
  m_thread.RunFullDiscovery(callback);
}

void OpenRDMOutputPort::RunIncrementalDiscovery(ola::rdm::RDMDiscoveryCallback *callback) {
  if (!m_rdm_enabled) {
    UIDSet uid_set;
    callback->Run(uid_set);
    return;
  }
  m_thread.RunIncrementalDiscovery(callback);
}

}  // namespace openrdm
}  // namespace plugin
}  // namespace ola