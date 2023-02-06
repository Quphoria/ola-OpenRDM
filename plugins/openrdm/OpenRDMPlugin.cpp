#include <vector>
#include <string>

#include "ola/StringUtils.h"
#include "olad/Preferences.h"
#include "olad/PluginAdaptor.h"
#include "plugins/openrdm/OpenRDMPlugin.h"
#include "plugins/openrdm/OpenRDMPluginDescription.h"
#include "plugins/openrdm/OpenRDMDevice.h"
#include "plugins/openrdm/OpenRDMWidget.h"

namespace ola {
namespace plugin {
namespace openrdm {

using std::string;
using std::vector;

const char OpenRDMPlugin::PLUGIN_NAME[] = "OpenRDM";
const char OpenRDMPlugin::PLUGIN_PREFIX[] = "openrdm";

const char OpenRDMPlugin::DEFAULT_PORT_DEVICE[] = "s:0x0403:0x6001:00418TL8";

const char OpenRDMPlugin::K_OUTPUT_PORTS[] = "output_ports";
const char OpenRDMPlugin::K_PORT_DEVICE_TEMPL[] = "port_%d_device";
const char OpenRDMPlugin::K_PORT_REFRESH_TEMPL[] = "port_%d_dmx_refresh_ms";
const char OpenRDMPlugin::K_PORT_RDM_TEMPL[] = "port_%d_rdm_enabled";

string ExpandTemplate(const string &str, unsigned int value);

/**
 * @brief Attempt to start a device and, if successful, register it
 *
 * Ownership of the OpenRDMDevice is transferred to us here.
 */
void OpenRDMPlugin::AddDevice(OpenRDMDevice *device) {
  if (device->Start()) {
      m_devices.push_back(device);
      m_plugin_adaptor->RegisterDevice(device);
  } else {
    OLA_WARN << "Failed to start OpenRDM device " << device->Description();
    delete device;
  }
}


/**
 * @brief Fetch a list of all OpenRDM devices and create a new device for each of them.
 */
bool OpenRDMPlugin::StartHook() {
  OpenRDMWidget::findDevices();

  auto dev_strings = std::vector<std::string>();
  for (unsigned int i = 0; i < GetOutputPortCount(); i++) {
    string dev_str = GetPortDeviceString(i);
    if (dev_str.size() > 0) {
      // Remove duplicate device strings (only use first occurence)
      for (auto & d : dev_strings) {
        if (dev_str.compare(d) == 0) {
          dev_str = "";
          OLA_WARN << "OpenRDM device string repeated, please ensure all device strings are unique" << strerror(errno);
          break;
        }
      }
    }
    dev_strings.push_back(dev_str);
  }

  for (unsigned int i = 0; i < dev_strings.size(); i++) {
    auto dev_str = dev_strings.at(i);
    if (dev_str.size()) {
      auto refresh_ms = GetPortRefreshMS(i);
      auto rdm_enabled = GetPortRDMEnabled(i);

      AddDevice(new OpenRDMDevice(this, i, dev_str, refresh_ms, rdm_enabled));
    }
  }

  return true;
}


/**
 * @brief Stop all the devices.
 */
bool OpenRDMPlugin::StopHook() {
  OpenRDMDeviceVector::iterator iter;
  for (iter = m_devices.begin(); iter != m_devices.end(); ++iter) {
    m_plugin_adaptor->UnregisterDevice(*iter);
    (*iter)->Stop();
    delete (*iter);
  }
  m_devices.clear();
  return true;
}


/**
 * @brief Return a description for this plugin.
 */
string OpenRDMPlugin::Description() const {
  return plugin_description;
}


/**
 * @brief Set the default preferences
 */
bool OpenRDMPlugin::SetDefaultPreferences() {
  if (!m_preferences) {
    return false;
  }

  bool save = false;

  save |= m_preferences->SetDefaultValue(K_OUTPUT_PORTS,
                                         UIntValidator(1, 32),
                                         DEFAULT_OUTPUT_PORTS);
  
  for (unsigned int i = 0; i < GetOutputPortCount(); i++) {
    save |= m_preferences->SetDefaultValue(ExpandTemplate(K_PORT_DEVICE_TEMPL, i),
                                           StringValidator(),
                                           DEFAULT_PORT_DEVICE);

    save |= m_preferences->SetDefaultValue(ExpandTemplate(K_PORT_REFRESH_TEMPL, i),
                                           UIntValidator(5, 1000*60),
                                           DEFAULT_REFRESH_MS);

    save |= m_preferences->SetDefaultValue(ExpandTemplate(K_PORT_RDM_TEMPL, i),
                                           BoolValidator(),
                                           DEFAULT_RDM_ENABLED);
  }

  if (save) {
    m_preferences->Save();
  }

  // check if this saved correctly
  // we don't want to use it if null
  unsigned int port_count; // prevent ToCToU error
  if (!StringToInt(m_preferences->GetValue(K_OUTPUT_PORTS), &port_count)) {
    return false;
  }

  for (unsigned int i = 0; i < port_count; i++) {
    if (m_preferences->GetValue(ExpandTemplate(K_PORT_DEVICE_TEMPL, i)).empty()) {
      return false;
    }

    if (m_preferences->GetValue(ExpandTemplate(K_PORT_REFRESH_TEMPL, i)).empty()) {
      return false;
    }

    if (m_preferences->GetValue(ExpandTemplate(K_PORT_RDM_TEMPL, i)).empty()) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Return the output port count from the preferences.
 *
 * Defaults to DEFAULT_OUTPUT_PORTS if the value was invalid.
 */
unsigned int OpenRDMPlugin::GetOutputPortCount() const {
  unsigned int port_count;
  if (!StringToInt(m_preferences->GetValue(K_OUTPUT_PORTS), &port_count)) {
    return DEFAULT_OUTPUT_PORTS;
  }
  return port_count;
}

/**
 * @brief Return the device string from the preferences for a specified port.
 */
string OpenRDMPlugin::GetPortDeviceString(unsigned int port) const {
  return m_preferences->GetValue(ExpandTemplate(K_PORT_DEVICE_TEMPL, port));
}

/**
 * @brief Return the DMX refresh milliseconds value from the preferences for a specified port.
 *
 * Defaults to DEFAULT_REFRESH_MS if the value was invalid.
 */
unsigned int OpenRDMPlugin::GetPortRefreshMS(unsigned int port) const {
  unsigned int refresh_ms;
  if (!StringToInt(m_preferences->GetValue(ExpandTemplate(K_PORT_REFRESH_TEMPL, port)), &refresh_ms)) {
    return DEFAULT_REFRESH_MS;
  }
  return refresh_ms;
}

/**
 * @brief Return if RDM is enabled from the preferences for a specified port.
 *
 * Defaults to DEFAULT_RDM_ENABLED if the value was invalid.
 */
bool OpenRDMPlugin::GetPortRDMEnabled(unsigned int port) const {
  bool rdm_enabled;
  if (!StringToBool(m_preferences->GetValue(ExpandTemplate(K_PORT_RDM_TEMPL, port)), &rdm_enabled)) {
    return DEFAULT_RDM_ENABLED;
  }
  return rdm_enabled;
}

string ExpandTemplate(const string &str, unsigned int value) {
  string output = str;
  // find the first instance of "%d" in the string.
  size_t pos = output.find("%d");
  if (pos != string::npos) {
    // %d was found, perform the replacement.
    output.replace(pos, 2, IntToString(value));
  }
  return output;
}
}  // namespace openrdm
}  // namespace plugin
}  // namespace ola