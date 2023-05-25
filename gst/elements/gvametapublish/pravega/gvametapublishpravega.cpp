//
// Copyright (c) Dell Inc., or its subsidiaries. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//

#include "gvametapublishpravega.hpp"

#include <common.hpp>
#include <safe_arithmetic.hpp>

extern "C" {
#include "pravega_client.h"
}

#include <cstdint>
#include <string>
#include <thread>

GST_DEBUG_CATEGORY_STATIC(gva_meta_publish_pravega_debug_category);
#define GST_CAT_DEFAULT gva_meta_publish_pravega_debug_category

/* Properties */
enum {
    PROP_0,
    PROP_CONTROLLER_URI,
    PROP_KEYCLOAK_FILE,
    PROP_AUTH_ENABLED,
    PROP_DISABLE_CERT,
    PROP_SCOPE,
    PROP_STREAM,
    PROP_ROUTING_KEY,
    // PROP_MAX_CONNECT_ATTEMPTS,
    // PROP_MAX_RECONNECT_INTERVAL,
};

class GvaMetaPublishPravegaPrivate {
  public:
    GvaMetaPublishPravegaPrivate(GvaMetaPublishBase *parent) : _base(parent) {}

    ~GvaMetaPublishPravegaPrivate() = default;

    gboolean start() {
        GST_INFO_OBJECT(_base,
                        "Gvametapushlish params: \n"
                        "--controller-uri: %s\n "
                        "--keycloak-file: %s\n "
                        "--auth-enabled: %s\n "
                        "--disable-cert: %s\n "
                        "--scope: %s\n "
                        "--stream: %s\n "
                        "--routing-key: %s\n",
                        _controller_uri.c_str(), _keycloak_file.c_str(), BOOL_TO_STR(_auth_enabled), BOOL_TO_STR(_disable_cert),
                        _scope.c_str(), _stream.c_str(), _routing_key.c_str());

        _stream_manager = create_stream_manager_with_keycloak(_controller_uri.c_str(), _keycloak_file.c_str(), _auth_enabled, _disable_cert);
        if (!_stream_manager) {
            GST_ERROR_OBJECT(_base, "Failed to create pravega stream manager.");
            return false;
        }

        if (stream_manager_create_scope(_stream_manager, _scope.c_str())) {
            GST_INFO_OBJECT(_base,"Scope:%s created.", _scope.c_str());
        } else {
            GST_INFO_OBJECT(_base,"Scope:%s exists or failed to create.", _scope.c_str());
        }
        if (stream_manager_create_stream(_stream_manager, _scope.c_str(), _stream.c_str(), 1)) {
            GST_INFO_OBJECT(_base,"Stream:%s/%s created.", _scope.c_str(), _stream.c_str());
        } else {
            GST_INFO_OBJECT(_base,"Stream:%s/%s created.", _scope.c_str(), _stream.c_str());
        }

        _stream_writer = create_stream_writer(_stream_manager, _scope.c_str(), _stream.c_str(), 20);
        if (!_stream_writer) {
            GST_ERROR_OBJECT(_base, "Failed to create pravega stream writer.");
            return false;
        }

        GST_DEBUG_OBJECT(_base, "Connect to pravega.");
        return true;
    }

    gboolean publish(const std::string &message) {
        // TODO Validate message is JSON
        int length = stream_writer_write_event(
            _stream_writer, 
            (const uint8_t*)(message.c_str()), 
            safe_convert<int>(message.size()),
            _routing_key.c_str()
        );

        if (length < 0 ) {
            GST_ERROR_OBJECT(_base, "Event failed to sent to Pravega.");
            return false;
        }
        GST_DEBUG_OBJECT(_base, "Pravega event sent.");
        return true;
    }

    gboolean stop() {
        if (!_stream_manager) {
            GST_DEBUG_OBJECT(_base, "Stream manager is not connected. Nothing to disconnect");
            return true;
        }
        if (!_stream_writer) {
            GST_DEBUG_OBJECT(_base, "Stream writer is not connected.");
        } else {
            if (stream_writer_flush(_stream_writer)) {
                GST_DEBUG_OBJECT(_base, "All events are flushed.");
            } else {
                GST_ERROR_OBJECT(_base, "Failed to flush.");
            }
            destroy_stream_writer(_stream_writer);
        }
        
        destroy_stream_manager(_stream_manager);
        GST_DEBUG_OBJECT(_base, "Disconnect to pravega.");
        return true;
    }

    bool set_property(guint prop_id, const GValue *value) {
        switch (prop_id) {
        case PROP_CONTROLLER_URI:
            _controller_uri = g_value_get_string(value);
            break;
        case PROP_KEYCLOAK_FILE:
            _keycloak_file = g_value_get_string(value);
            break;
        case PROP_AUTH_ENABLED:
            _auth_enabled = g_value_get_boolean(value);
            break;
        case PROP_DISABLE_CERT:
            _disable_cert = g_value_get_boolean(value);
            break;
        case PROP_SCOPE:
            _scope = g_value_get_string(value);
            break;
        case PROP_STREAM:
            _stream = g_value_get_string(value);
            break;
        case PROP_ROUTING_KEY:
            _routing_key = g_value_get_string(value);
            break;
        // case PROP_MAX_CONNECT_ATTEMPTS:
        //     _max_connect_attempts = g_value_get_uint(value);
        //     break;
        // case PROP_MAX_RECONNECT_INTERVAL:
        //     _max_reconnect_interval = g_value_get_uint(value);
        //     break;
        default:
            return false;
        }

        return true;
    }

    bool get_property(guint prop_id, GValue *value) {
        switch (prop_id) {
        case PROP_CONTROLLER_URI:
            g_value_set_string(value, _controller_uri.c_str());
            break;
        case PROP_KEYCLOAK_FILE:
            g_value_set_string(value, _keycloak_file.c_str());
            break;
        case PROP_AUTH_ENABLED:
            g_value_set_boolean(value, _auth_enabled);
            break;
        case PROP_DISABLE_CERT:
            g_value_set_boolean(value, _disable_cert);
            break;
        case PROP_SCOPE:
            g_value_set_string(value, _scope.c_str());
            break;
        case PROP_STREAM:
            g_value_set_string(value, _stream.c_str());
            break;
        case PROP_ROUTING_KEY:
            g_value_set_string(value, _routing_key.c_str());
            break;
        // case PROP_MAX_CONNECT_ATTEMPTS:
        //     g_value_set_uint(value, _max_connect_attempts);
        //     break;
        // case PROP_MAX_RECONNECT_INTERVAL:
        //     g_value_set_uint(value, _max_reconnect_interval);
        //     break;
        default:
            return false;
        }
        return true;
    }

  private:
    GvaMetaPublishBase *_base;

    std::string _controller_uri;
    std::string _keycloak_file;
    std::string _scope;
    std::string _stream;
    std::string _routing_key;
    // uint32_t _max_connect_attempts;
    // uint32_t _max_reconnect_interval;
    bool _auth_enabled;
    bool _disable_cert;

    StreamManager *_stream_manager;
    StreamWriter  *_stream_writer;
};

G_DEFINE_TYPE_EXTENDED(GvaMetaPublishPravega, gva_meta_publish_pravega, GST_TYPE_GVA_META_PUBLISH_BASE, 0,
                       G_ADD_PRIVATE(GvaMetaPublishPravega);
                       GST_DEBUG_CATEGORY_INIT(gva_meta_publish_pravega_debug_category, "gvametapublishpravega", 0,
                                               "debug category for gvametapublishpravega element"));

static void gva_meta_publish_pravega_init(GvaMetaPublishPravega *self) {
    // Initialize of private data
    auto *priv_memory = gva_meta_publish_pravega_get_instance_private(self);
    self->impl = new (priv_memory) GvaMetaPublishPravegaPrivate(&self->base);
}

static void gva_meta_publish_pravega_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    auto self = GVA_META_PUBLISH_PRAVEGA(object);

    if (!self->impl->get_property(prop_id, value))
        G_OBJECT_CLASS(gva_meta_publish_pravega_parent_class)->get_property(object, prop_id, value, pspec);
}

static void gva_meta_publish_pravega_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    auto self = GVA_META_PUBLISH_PRAVEGA(object);

    if (!self->impl->set_property(prop_id, value))
        G_OBJECT_CLASS(gva_meta_publish_pravega_parent_class)->set_property(object, prop_id, value, pspec);
}

static void gva_meta_publish_pravega_finalize(GObject *object) {
    auto self = GVA_META_PUBLISH_PRAVEGA(object);
    g_assert(self->impl && "Expected valid 'impl' pointer during finalize");

    if (self->impl) {
        self->impl->~GvaMetaPublishPravegaPrivate();
        self->impl = nullptr;
    }

    G_OBJECT_CLASS(gva_meta_publish_pravega_parent_class)->finalize(object);
}

static void gva_meta_publish_pravega_class_init(GvaMetaPublishPravegaClass *klass) {
    auto gobject_class = G_OBJECT_CLASS(klass);
    auto base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);
    auto base_metapublish_class = GVA_META_PUBLISH_BASE_CLASS(klass);

    gobject_class->set_property = gva_meta_publish_pravega_set_property;
    gobject_class->get_property = gva_meta_publish_pravega_get_property;
    gobject_class->finalize = gva_meta_publish_pravega_finalize;

    base_transform_class->start = [](GstBaseTransform *base) { return GVA_META_PUBLISH_PRAVEGA(base)->impl->start(); };
    base_transform_class->stop = [](GstBaseTransform *base) { return GVA_META_PUBLISH_PRAVEGA(base)->impl->stop(); };

    base_metapublish_class->publish = [](GvaMetaPublishBase *base, const std::string &message) {
        return GVA_META_PUBLISH_PRAVEGA(base)->impl->publish(message);
    };

    gst_element_class_set_static_metadata(GST_ELEMENT_CLASS(klass), "Pravega metadata publisher", "Metadata",
                                          "Publishes the JSON metadata to Pravega event stream", "Intel Corporation && DellEMC Corporation");

    auto prm_flags = static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);
    g_object_class_install_property(
        gobject_class, PROP_CONTROLLER_URI,
        g_param_spec_string("controller-uri",
                            "Pravega Controller Uri",
                            "Pravega Controller Uri",
                            DEFAULT_CONTROLLER_URI, prm_flags));
    g_object_class_install_property(
        gobject_class, PROP_KEYCLOAK_FILE,
        g_param_spec_string("keycloak-file",
                            "Keycloak File",
                            "Keycloak File",
                            DEFAULT_KEYCLOAK_FILE, prm_flags));
    g_object_class_install_property(
        gobject_class, PROP_AUTH_ENABLED,
        g_param_spec_boolean("auth-enabled",
                            "Auth Enabled",
                            "Auth Enabled",
                            DEFAULT_AUTH_ENABLED, prm_flags));
    g_object_class_install_property(
        gobject_class, PROP_DISABLE_CERT,
        g_param_spec_boolean("disable-cert",
                            "Disable Cert",
                            "Disable Cert Verification",
                            DEFAULT_DISABLE_CERT, prm_flags));
    g_object_class_install_property(
        gobject_class, PROP_SCOPE,
        g_param_spec_string("scope",
                            "Scope",
                            "Scope in which to create pravega stream",
                            DEFAULT_SCOPE, prm_flags));
    g_object_class_install_property(
        gobject_class, PROP_STREAM,
        g_param_spec_string("stream",
                            "Stream",
                            "Stream to which to send pravega events",
                            DEFAULT_STREAM, prm_flags));
    g_object_class_install_property(
        gobject_class, PROP_ROUTING_KEY,
        g_param_spec_string("routing-key",
                            "Routing Key",
                            "Routing Key",
                            DEFAULT_ROUTING_KEY, prm_flags));
    // g_object_class_install_property(
    //     gobject_class, PROP_MAX_CONNECT_ATTEMPTS,
    //     g_param_spec_uint("max-connect-attempts",
    //                       "Max Connect Attempts",
    //                       "Maximum number of failed connection attempts before it is considered fatal.",
    //                       1, 10, DEFAULT_MAX_CONNECT_ATTEMPTS, prm_flags));
    // g_object_class_install_property(
    //     gobject_class, PROP_MAX_RECONNECT_INTERVAL,
    //     g_param_spec_uint("max-reconnect-interval",
    //                       "Max Reconnect Interval",
    //                       "Maximum time in seconds between reconnection attempts. Initial interval is 1 second and will be doubled on each failure up to this maximum interval.",
    //                       1, 300, DEFAULT_MAX_RECONNECT_INTERVAL, prm_flags));
}

static gboolean plugin_init(GstPlugin *plugin) {
    gboolean result = TRUE;
    result &= gst_element_register(plugin, "gvametapublishpravega", GST_RANK_NONE, GST_TYPE_GVA_META_PUBLISH_PRAVEGA);
    return result;
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, gvametapublishpravega,
                  PRODUCT_FULL_NAME " PRAVEGA metapublish element", plugin_init, PLUGIN_VERSION, PLUGIN_LICENSE,
                  PACKAGE_NAME, GST_PACKAGE_ORIGIN)
