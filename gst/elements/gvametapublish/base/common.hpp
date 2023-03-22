/*******************************************************************************
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <gst/gst.h>

GST_EXPORT GstStaticPadTemplate gva_meta_publish_sink_template;
GST_EXPORT GstStaticPadTemplate gva_meta_publish_src_template;

typedef enum { GVA_META_PUBLISH_JSON = 1, GVA_META_PUBLISH_JSON_LINES = 2 } FileFormat;

// File specific constants
constexpr auto STDOUT = "stdout";
constexpr auto DEFAULT_FILE_PATH = STDOUT;
constexpr auto DEFAULT_FILE_FORMAT = GVA_META_PUBLISH_JSON;

// Enum value names
constexpr auto UNKNOWN_VALUE_NAME = "unknown";

constexpr auto PUBLISH_METHOD_FILE_NAME = "file";
constexpr auto PUBLISH_METHOD_MQTT_NAME = "mqtt";
constexpr auto PUBLISH_METHOD_KAFKA_NAME = "kafka";
constexpr auto PUBLISH_METHOD_PRAVEGA_NAME = "pravega";

constexpr auto FILE_FORMAT_JSON_NAME = "json";
constexpr auto FILE_FORMAT_JSON_LINES_NAME = "json-lines";

// Broker specific constants
constexpr auto DEFAULT_ADDRESS = "";
constexpr auto DEFAULT_MQTTCLIENTID = "";
constexpr auto DEFAULT_TOPIC = "";
constexpr auto DEFAULT_SIGNAL_HANDOFFS = false;
constexpr auto DEFAULT_MAX_CONNECT_ATTEMPTS = 1;
constexpr auto DEFAULT_MAX_RECONNECT_INTERVAL = 30;
constexpr auto DEFAULT_CONTROLLER_URI = "tcp://127.0.0.1:9090";
constexpr auto DEFAULT_KEYCLOCK_FILE = "";
constexpr auto DEFAULT_AUTH_ENABLED = false;
constexpr auto DEFAULT_DISABLE_CERT = false;
constexpr auto DEFAULT_SCOPE = "";
constexpr auto DEFAULT_STREAM = "";
constexpr auto DEFAULT_ROUTING_KEY = "";

GST_EXPORT const gchar *file_format_to_string(FileFormat format);

GST_EXPORT GType gva_metapublish_file_format_get_type(void);
#define GST_TYPE_GVA_METAPUBLISH_FILE_FORMAT (gva_metapublish_file_format_get_type())

#define BOOL_TO_STR(b) ((b) ? "true" : "false")