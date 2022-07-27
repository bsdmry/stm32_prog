#ifndef H_IF_AUDIOCTL
#define H_IF_AUDIOCTL

extern const struct usb_interface_descriptor audio_control_iface[];

 struct func_descr {
    struct usb_audio_header_descriptor_head header_head;
    struct usb_audio_header_descriptor_body header_body;
    struct usb_audio_input_terminal_descriptor input_terminal_desc;
    struct usb_audio_feature_unit_descriptor_2ch feature_unit_desc;
    struct usb_audio_output_terminal_descriptor output_terminal_desc;
 } __attribute__((packed)); 

 extern const struct func_descr audio_control_functional_descriptors;

#endif
