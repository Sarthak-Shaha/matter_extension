/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ATTRIBUTE_TRANSLATOR_INTERFACE_HPP
#define ATTRIBUTE_TRANSLATOR_INTERFACE_HPP

#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <string>

// Matter library
#include "matter.h"

// Matter bridge components
#include "matter_device_translator.hpp"
#include "matter_node_state_monitor.hpp"
#include "unify_mqtt_wrapper.hpp"

// Unify SDK
#include "sl_log.h"
#include "uic_mqtt.h"

namespace unify::matter_bridge {

/**
 * @brief Base class for attribute access interfaces
 *
 * This class performs the base functionality for all attribute access
 * interfaces for in the Unify Bridge. Upon construction the class will register
 * itself with chip::app framework
 *
 */
class attribute_translator_interface : public chip::app::AttributeAccessInterface
{
public:
    attribute_translator_interface(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                   device_translator & dev_translator, chip::ClusterId id, const char * log_tag) :
        chip::app::AttributeAccessInterface(chip::Optional<chip::EndpointId>::Missing(), id),
        m_node_state_monitor(node_state_monitor), LOG_TAG(log_tag), m_unify_mqtt(unify_mqtt), m_dev_translator(dev_translator)
    {
        registerAttributeAccessOverride(this);

        // Register the an event listener for subscriptions
        auto f = [&](const bridged_endpoint & ep, matter_node_state_monitor::update_t update) {
            attributes_update_subscription(ep, update);
        };
        node_state_monitor.register_event_listener(f);
    }

protected:
    /**
     * @brief List of cluster names which this translator will be using.
     *
     * @return std::vector<const char *>
     */
    virtual std::vector<const char *> unify_cluster_names() const = 0;

    /**
     * @brief Called when a reported attribute is updated
     *
     * @param ep
     * @param cluster
     * @param attribute
     * @param value
     */
    virtual void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                                  const nlohmann::json & value) = 0;
    matter_node_state_monitor & m_node_state_monitor;

private:
    const char * LOG_TAG;

    void on_mqtt_message_cb(const char * topic, const char * message, const size_t message_length)
    {
        std::regex rgx("ucl/by-unid"
                       "/([^/]*)"   // UNID
                       "/ep([^/]*)" // Endpoint
                       "/([^/]*)"   // Cluster
                       "/Attributes"
                       "/([^/]*)" // Attribute
                       "/Reported");
        std::smatch match;
        std::string topic_str(topic);
        if (!std::regex_search(topic_str, match, rgx))
        {
            return;
        }
        const std::string & unid        = match.str(1);
        const std::string & endpoint_id = match.str(2);
        const std::string & cluster     = match.str(3);
        const std::string & attribute   = match.str(4);

        auto unify_node = m_node_state_monitor.bridged_endpoint(unid, std::stoi(endpoint_id));
        // In Matter Bridge Endpoint 0 is dedicated to the root node (bridge app)
        // So unify bridged node will not be assigned endpoint 0.
        if (!unify_node)
        {
            sl_log_debug(LOG_TAG, "The bridged node is not assigned a matter dynamic endpoint\n");
            return;
        }

        std::string msg(message, message_length);
        if (!attribute.empty() && !cluster.empty() && !msg.empty())
        {
            try
            {
                nlohmann::json jsn = nlohmann::json::parse(msg);
                reported_updated(unify_node, cluster, attribute, jsn["value"]);
            } catch (const nlohmann::json::parse_error & e)
            {
                sl_log_warning(LOG_TAG,
                               "Error parsing attribute %s reported value for %s/ep%s "
                               "as the payload does not appear to be correct JSON, %s\n",
                               attribute.c_str(), unid.c_str(), endpoint_id.c_str(),
                               e.what());
            } catch (const nlohmann::json::type_error & e)
            {
                sl_log_warning(LOG_TAG,
                               "Error parsing attribute %s reported value for %s/ep%s "
                               "as the value is of different type or key is unknown to us, %s\n%s\n",
                               attribute.c_str(), unid.c_str(), endpoint_id.c_str(),
                               e.what(), msg.c_str());
            }
        }
        else
        {
            sl_log_debug(LOG_TAG, "Unknown attributes [%s]", attribute.c_str());
        }
    }

    void attributes_update_subscription(const bridged_endpoint & ep, matter_node_state_monitor::update_t update)
    {
        sl_log_debug(LOG_TAG, "Matter endpoint: %d", ep.matter_endpoint);
        for (const auto & unify_cluster : unify_cluster_names())
        {
            std::string topic = "ucl/by-unid/" + ep.unify_unid + "/ep" + std::to_string(ep.unify_endpoint) + "/" + unify_cluster +
                "/Attributes/+/Reported";

            if (update == matter_node_state_monitor::update_t::NODE_ADDED)
            {
                m_unify_mqtt.SubscribeEx(topic.c_str(), attribute_translator_interface::on_mqtt_message_c_cb, this);
            }
            else if (update == matter_node_state_monitor::update_t::NODE_DELETED)
            {
                m_unify_mqtt.UnsubscribeEx(topic.c_str(), attribute_translator_interface::on_mqtt_message_c_cb, this);
            }
        }
    }

    static void on_mqtt_message_c_cb(const char * topic, const char * message, const size_t message_length, void * user)
    {
        attribute_translator_interface * instance = static_cast<attribute_translator_interface *>(user);
        if (instance)
        {
            instance->on_mqtt_message_cb(topic, message, message_length);
        }
    }

    UnifyMqtt & m_unify_mqtt;
    device_translator & m_dev_translator;
};
} // namespace unify::matter_bridge

#endif // ATTRIBUTE_TRANSLATOR_INTERFACE_HPP
