# Custom Matter Device Development

Build a customizable lighting app using the Matter protocol.

## Overview

This guide covers the basics of building a customizable lighting application
using Matter.

## Using Matter with Clusters

In Matter, commands can be issued by using a cluster. A cluster is a set of
attributes and commands which are grouped together under a relevant theme.

Attributes store values (think of them as variables). Commands are used to
modify the value of attributes.

For example, the "On/Off" cluster has an attribute named "OnOff" of type
boolean. The value of this attribute can be set to "1" by sending an "On"
command or it can be set to "0" by sending an "Off" command.

The C++ implementation of these clusters is located in the clusters directory.
Note that you can also create your own custom cluster.

## ZAP Configuration

From the matter repository, run the following command in a terminal to launch
the ZAP user interface (UI). This will open up the ZAP configuration for the EFR32 lighting
application example.

```shell
$ ./scripts/tools/zap/run_zaptool.sh examples/lighting-app/lighting-common/lighting-app.zap
```
On the left side of the application, there is a tab for Endpoint 0 and
Endpoint 1. Endpoint 0 is known as the root node. This endpoint is akin to a
"read me first" endpoint that describes itself and the other endpoints that make
up the node. Endpoint 1 represents a lighting application device type. There are
a number of required ZCL clusters enabled in Endpoint 1. Some clusters are
common across most device types, such as identify and group clusters. Others,
such as the On/Off, Level Control and Color Control clusters are required for a
lighting application device type.

Clicking on the blue settings icon on the right side of the application
brings you to the zap configuration settings for that cluster. Each cluster
has some required attributes that may cause compile-time errors if they are not
selected in the zap configuration. Other attributes are optional and are allowed
to be disabled. Clusters also have a list of client-side commands, again some
are mandatory and others are optional depending on the cluster. ZCL offers an
extensive list of optional attributes and commands that allow you to customize
your application to the full power of the Matter SDK.

For example, if a lighting application only includes
single color LEDs instead of RGB LEDs, it might make sense to disable the Color
Control cluster in the ZAP configuration. Similarly, if a
lighting application does not take advantage of the Level Control cluster,
which allows you to customize current flow to an LED, it might make sense to
disable the Level Control cluster.

Each time a modification is made to the ZAP UI, save (Electron→Save on
a Mac toolbar) the current ZAP configuration and run the following command to
generate ZAP code.

```shell
$ ./scripts/tools/zap/generate.py examples/lighting-app/lighting-common/lighting-app.zap -o zzz_generated/lighting-app/zap-generated/
```
## Receiving Matter Commands

All Matter commands reach the application through the intermediate function
`MatterPostAttributeChangeCallback()`. When a request is made by a Matter client,
the information contained in the request is forwarded to a Matter application
through this function. The command can then be dissected using conditional logic
to call the proper application functions based on the most recent command
received.

## Adding a Cluster to a ZAP Configuration

In the ZAP UI, navigate to the Level Control cluster. Make sure this cluster is
enabled as a server in the drop-down menu in the "Enable" column. Then click on
the blue settings wheel in the "Configure" column. This cluster can be used to
gather power source configuration settings from a Matter device. It contains a
few required attributes, and a number of optional attributes.

## Adding a New Attribute

In the Level Control cluster configurations, ensure the CurrentLevel attribute
is set to enabled. Set the default value of this attribute as 1.

## Adding a New Command

Navigate to the commands tab in zap and enable the MoveToLevel command. Now save
the current zap configuration, and run the generate.py script above.

## React to Level Control Cluster Commands in ZclCallbacks

In the MatterPostAttributeCallback function in ZclCallbacks, add the following
line of code or a similar line. This will give the application the ability to react to
MoveToLevel commands. You can define platform-specific behavior for a
MoveToLevel action.

   ```cpp
    else if (clusterId == LevelControl::Id)
    {
       ChipLogProgress(Zcl, "Level Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);

       if (attributeId == LevelControl::Attributes::CurrentLevel::Id)
       {
          action_type = LightingManager::MOVE_TO_LEVEL;
       }

       LightMgr().InitiateActionLight(AppEvent::kEventType_Light, action_type, endpoint, *value);
    }
   ```

## Send a MoveToLevel Command and Read the CurrentLevel Attribute

Rebuild the application and load the new executable on your EFR32 device. Send
the following mattertool commands and verify that the current-level default
attribute was updated as was configured. Replace {desired_level} with 10, and
node_ID with the node ID assigned to the device upon commissioning.

```shell
$ mattertool levelcontrol read current-level 1 1 // Returns 1
```

```shell
$ mattertool levelcontrol move-to-level {desired_level} 0 1 1 {node_ID} 1
```

```shell
$ mattertool levelcontrol read current-level 1 1 // Returns 10
```

For more information on running a Silicon Labs lighting example on a Thunderboard Sense 2 see 
[sl-newlight/efr32](https://github.com/SiliconLabs/matter/blob/latest/silabs_examples/sl-newLight/efr32/README.md). 
