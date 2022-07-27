#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/audio.h>
#include "if_audiostream.h"

const struct usb_interface_descriptor audio_streaming_iface[] = { {
    /* zerobw streaming interface (alt 0) */
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 1,
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = USB_CLASS_AUDIO,
    .bInterfaceSubClass = USB_AUDIO_SUBCLASS_AUDIOSTREAMING,
    .bInterfaceProtocol = 0,
    .iInterface = 0,

    .extra = 0,
    .extralen = 0,
}, {
    /* Actual streaming interface (alt 1) */
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 1,
    .bAlternateSetting = 1,
    .bNumEndpoints = 1,
    .bInterfaceClass = USB_CLASS_AUDIO,
    .bInterfaceSubClass = USB_AUDIO_SUBCLASS_AUDIOSTREAMING,
    .bInterfaceProtocol = 0,
    .iInterface = 0,

    .endpoint = isochronous_ep,

    .extra = &audio_streaming_functional_descriptors,
    .extralen = sizeof(audio_streaming_functional_descriptors)
} };

const struct stream_descr audio_streaming_functional_descriptors = {
    .audio_cs_streaming_iface_desc = {
        .bLength = sizeof(struct usb_audio_stream_interface_descriptor),
        .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
        .bDescriptorSubtype = 1, /* AS_GENERAL */
        .bTerminalLink = 3, /* Terminal 3 */
        .bDelay = 0,
        .wFormatTag = 0x0001 /* PCM Format */,
    },
    .audio_type1_format_desc = {
        .head = {
            .bLength = sizeof(struct usb_audio_format_type1_descriptor_1freq),
            .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
            .bDescriptorSubtype = 2, /* FORMAT_TYPE */
            .bFormatType = 1,
            .bNrChannels = 2,
            .bSubFrameSize = 2,
            .bBitResolution = 16, /* Should be 10, but 16 is more reliable */
            .bSamFreqType = 1, /* 1 discrete sampling frequency */
        },
        .freqs = { {
            .tSamFreq = SAMPLE_RATE,
        } },
    }
};

const struct usb_audio_stream_audio_endpoint_descriptor audio_streaming_cs_ep_desc[] = { {
    .bLength = sizeof(struct usb_audio_stream_audio_endpoint_descriptor),
    .bDescriptorType = USB_AUDIO_DT_CS_ENDPOINT,
    .bDescriptorSubtype = 1, /* EP_GENERAL */
    .bmAttributes = 0,
    .bLockDelayUnits = 0x02, /* PCM samples */
    .wLockDelay = 0x0000,
} };

const struct usb_endpoint_descriptor isochronous_ep[] = { {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x82,
    .bmAttributes = USB_ENDPOINT_ATTR_ASYNC | USB_ENDPOINT_ATTR_ISOCHRONOUS,
    .wMaxPacketSize = 256,
    .bInterval = 0x01, /* 1 millisecond */

    /* not using usb_audio_stream_endpoint_descriptor??
     * (Why? These are USBv1.0 endpoint descriptors)*/

    .extra = &audio_streaming_cs_ep_desc[0],
    .extralen = sizeof(audio_streaming_cs_ep_desc[0])
} };


