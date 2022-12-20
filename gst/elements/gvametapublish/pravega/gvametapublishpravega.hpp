//
// Copyright (c) Dell Inc., or its subsidiaries. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//

#pragma once

#include <gvametapublishbase.hpp>

G_BEGIN_DECLS

#define GST_TYPE_GVA_META_PUBLISH_PRAVEGA (gva_meta_publish_pravega_get_type())
#define GVA_META_PUBLISH_PRAVEGA(obj)                                                                                     \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_GVA_META_PUBLISH_PRAVEGA, GvaMetaPublishPravega))
#define GVA_META_PUBLISH_PRAVEGA_CLASS(klass)                                                                             \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_GVA_META_PUBLISH_PRAVEGA, GvaMetaPublishPravegaClass))
#define IS_GVA_META_PUBLISH_PRAVEGA(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_GVA_META_PUBLISH_PRAVEGA))
#define IS_GVA_META_PUBLISH_PRAVEGA_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_GVA_META_PUBLISH_PRAVEGA))
#define GVA_META_PUBLISH_PRAVEGA_GET_CLASS(obj)                                                                           \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GST_TYPE_GVA_META_PUBLISH_PRAVEGA, GvaMetaPublishPravegaClass))

struct GvaMetaPublishPravega {
    GvaMetaPublishBase base;

    class GvaMetaPublishPravegaPrivate *impl;
};

struct GvaMetaPublishPravegaClass {
    GvaMetaPublishBaseClass base;
};

GType gva_meta_publish_pravega_get_type(void);

G_END_DECLS
