#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/audio.h>
#include "if_audiocontrol.h"

const struct usb_interface_descriptor audio_control_iface[] = {{
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = USB_CLASS_AUDIO,
    .bInterfaceSubClass = USB_AUDIO_SUBCLASS_CONTROL,
    .bInterfaceProtocol = 0,
    .iInterface = 0,

    .extra = &audio_control_functional_descriptors,
    .extralen = sizeof(audio_control_functional_descriptors)
} };

const struct func_descr audio_control_functional_descriptors = {
    .header_head = {
        .bLength = sizeof(struct usb_audio_header_descriptor_head) +
                   1 * sizeof(struct usb_audio_header_descriptor_body),
        .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
        .bDescriptorSubtype = USB_AUDIO_TYPE_HEADER,
        .bcdADC = 0x0100,
        .wTotalLength =
               sizeof(struct usb_audio_header_descriptor_head) +
               1 * sizeof(struct usb_audio_header_descriptor_body) +
               sizeof(struct usb_audio_input_terminal_descriptor) +
               sizeof(struct usb_audio_feature_unit_descriptor_2ch) +
               sizeof(struct usb_audio_output_terminal_descriptor),
        .binCollection = 1,
    },
    .header_body = {
        .baInterfaceNr = 0x01,
    },
    .input_terminal_desc = {
        .bLength = sizeof(struct usb_audio_input_terminal_descriptor),
        .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
        .bDescriptorSubtype = USB_AUDIO_TYPE_INPUT_TERMINAL,
        .bTerminalID = 1,
        .wTerminalType = 0x0710, /* Radio receiver */
        .bAssocTerminal = 0,
        .bNrChannels = 2,
        .wChannelConfig = 0x0003, /* Left & Right channels */
        .iChannelNames = 0,
        .iTerminal = 0,
    },
    .feature_unit_desc = {
        .head = {
            .bLength = sizeof(struct usb_audio_feature_unit_descriptor_2ch),
            .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
            .bDescriptorSubtype = USB_AUDIO_TYPE_FEATURE_UNIT,
            .bUnitID = 2,
            .bSourceID = 1, /* Input terminal 1 */
            .bControlSize = 2,
            .bmaControlMaster = 0x0001, /* 'Mute' is supported */
        },
        .channel_control = { {
            .bmaControl = 0x0000,
        }, {
            .bmaControl = 0x0000,
        } },
        .tail = {
            .iFeature = 0x00,
        }
    },
    .output_terminal_desc = {
        .bLength = sizeof(struct usb_audio_output_terminal_descriptor),
        .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
        .bDescriptorSubtype = USB_AUDIO_TYPE_OUTPUT_TERMINAL,
        .bTerminalID = 3,
        .wTerminalType = 0x0101, /* USB Streaming */
        .bAssocTerminal = 0,
        .bSourceID = 0x02, /* Feature unit 2 */
        .iTerminal = 0,
    }
};
