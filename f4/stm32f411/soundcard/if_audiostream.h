#ifndef H_IF_AUDIOSTREAM
#define H_IF_AUDIOSTREAM

#define SAMPLE_RATE         8000
extern const struct usb_interface_descriptor audio_streaming_iface[];


struct stream_descr {
    struct usb_audio_stream_interface_descriptor audio_cs_streaming_iface_desc;
    struct usb_audio_format_type1_descriptor_1freq audio_type1_format_desc;
} __attribute__((packed));


extern const struct stream_descr audio_streaming_functional_descriptors;
extern const struct usb_audio_stream_audio_endpoint_descriptor audio_streaming_cs_ep_desc[];
extern const struct usb_endpoint_descriptor isochronous_ep[];

#endif
