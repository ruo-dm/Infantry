/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: Recieve.proto */

#ifndef PROTOBUF_C_Recieve_2eproto__INCLUDED
#define PROTOBUF_C_Recieve_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003003 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _DeviceToHost__Frame DeviceToHost__Frame;


/* --- enums --- */


/* --- messages --- */

struct  _DeviceToHost__Frame
{
  ProtobufCMessage base;
  float current_pitch_;
  float current_yaw_;
  int32_t current_color_;
  float bullet_speed_;
  int32_t mode_;
};
#define DEVICE_TO_HOST__FRAME__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&device_to_host__frame__descriptor) \
    , 0, 0, 0, 0, 0 }


/* DeviceToHost__Frame methods */
void   device_to_host__frame__init
                     (DeviceToHost__Frame         *message);
size_t device_to_host__frame__get_packed_size
                     (const DeviceToHost__Frame   *message);
size_t device_to_host__frame__pack
                     (const DeviceToHost__Frame   *message,
                      uint8_t             *out);
size_t device_to_host__frame__pack_to_buffer
                     (const DeviceToHost__Frame   *message,
                      ProtobufCBuffer     *buffer);
DeviceToHost__Frame *
       device_to_host__frame__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   device_to_host__frame__free_unpacked
                     (DeviceToHost__Frame *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*DeviceToHost__Frame_Closure)
                 (const DeviceToHost__Frame *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor device_to_host__frame__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_Recieve_2eproto__INCLUDED */
