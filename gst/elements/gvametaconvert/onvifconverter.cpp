/*******************************************************************************
 * Copyright (C) 2018-2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "onvifconverter.h"
#include "gva_utils.h"
#include "video_frame.h"
#include <utils.h>

#ifdef AUDIO
#include "audioconverter.h"
#endif
#include "convert_tensor.h"

#include <nlohmann/json.hpp>

#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>

using json = nlohmann::json;

GST_DEBUG_CATEGORY_STATIC(gst_onvif_converter_debug);
#define GST_CAT_DEFAULT gst_onvif_converter_debug

#define UTC_TO_TAI_SECONDS 37;

namespace {
/**
 * @return JSON object which contains parameters such as resolution, timestamp, source and tags.
 */
json get_frame_data(GstGvaMetaConvert *converter, GstBuffer *buffer) {
    assert(converter && buffer && "Expected valid pointers GstGvaMetaConvert and GstBuffer");

    json res;

    // Convert timestamp from TAI to UTC in onvif format
    GstClockTime tai_pts = buffer->pts;
    if (tai_pts != G_MAXUINT64) {
        std::time_t tai_time_seconds = tai_pts / std::pow(10,9);
        std::time_t utc_time_seconds = tai_time_seconds - UTC_TO_TAI_SECONDS;
        std::tm* utc_time = std::gmtime(&utc_time_seconds);
        int milli_seconds = (tai_pts / 1'000'000) % 1'000;
        std::stringstream ss;
        ss << std::put_time(utc_time, "%Y-%m-%dT%H:%M:%S.") << std::setfill('0') << std::setw(3) << milli_seconds;
        res["@UtcTime"] = ss.str();
    }

    if (converter->info) {
        // res["Transformation"] = {
        //     {"Translate", {{"@x", -1.0}, {"@y", -1.0}}},
        //     {"Scale", {{"@x", converter->info->width}, {"@y", converter->info->height}}},
        // };
    }
    return res;
}

/**
 * @return JSON array which contains ROIs attributes and their detection results.
 * Also contains ROIs classification results if any.
 */
json convert_roi_detection(GstGvaMetaConvert *converter, GstBuffer *buffer) {
    assert(converter && buffer && "Expected valid pointers GstGvaMetaConvert and GstBuffer");

    json res;
    GVA::VideoFrame video_frame(buffer, converter->info);
    for (GVA::RegionOfInterest &roi : video_frame.regions()) {
        // skip roi objects.
        const gchar *roi_type = g_quark_to_string(roi._meta()->roi_type);
        if (strcmp(roi_type, "roi") == 0) {
            continue;
        }

        json jobject = json::object();
        
        // object id
        gint id = 0;
        get_object_id(roi._meta(), &id);
        if (id != 0)
            jobject.push_back({"@ObjectId", id});

        guint x = roi._meta()->x;
        guint y = roi._meta()->y;
        guint w = roi._meta()->w;
        guint h = roi._meta()->h;
        for (GList *l = roi._meta()->params; l; l = g_list_next(l)) {
            json japperance;
            GstStructure *s = GST_STRUCTURE(l->data);
            const gchar *s_name = gst_structure_get_name(s);
            if (strcmp(s_name, "detection") == 0) {
                double xminval;
                double xmaxval;
                double yminval;
                double ymaxval;
                double confidence;
                if (gst_structure_get(s, "x_min", G_TYPE_DOUBLE, &xminval, "x_max", G_TYPE_DOUBLE, &xmaxval, "y_min",
                                      G_TYPE_DOUBLE, &yminval, "y_max", G_TYPE_DOUBLE, &ymaxval, NULL)) {
                    // from bbox coord to roi coord
                    double left = xminval
                    json jshape = {
                        {"BoundingBox",
                         {{"@left", xminval}, {"@right", xmaxval}, {"@bottom", yminval}, {"@top", ymaxval}}}};
                    japperance["Shape"] = jshape;

                    json jclass;
                    const gchar *label = g_quark_to_string(roi._meta()->roi_type);
                    if (label && gst_structure_get(s, "confidence", G_TYPE_DOUBLE, &confidence, NULL)) {
                        json jtype = {
                            {"#text", label},
                            {"@Likelihood", confidence},
                        };
                        jclass["Type"].push_back(jtype);
                    }
                    japperance["Class"] = jclass;
                }
            } else {
                char *label;
                char *model_name;
                if (gst_structure_get(s, "label", G_TYPE_STRING, &label, "model_name", G_TYPE_STRING, &model_name,
                                      NULL)) {
                    double confidence;
                    int label_id;
                    const gchar *attribute_name = gst_structure_has_field(s, "attribute_name")
                                                      ? gst_structure_get_string(s, "attribute_name")
                                                      : s_name;
                    json classification = json::object({{"label", label}, {"model", {{"name", model_name}}}});

                    if (gst_structure_get(s, "confidence", G_TYPE_DOUBLE, &confidence, NULL)) {
                        classification.push_back({"confidence", confidence});
                    }

                    if (gst_structure_get(s, "label_id", G_TYPE_INT, &label_id, NULL)) {
                        classification.push_back({"label_id", label_id});
                    }

                    japperance.push_back(json::object_t::value_type(attribute_name, classification));
                    g_free(label);
                    g_free(model_name);
                }
            }
            if (!japperance.empty()) {
                jobject.push_back({"Apperance", japperance});
            }
        }
        if (!jobject.empty()) {
            res.push_back(jobject);
        }
    }
    return res;
}

} // namespace

gboolean to_onvif_json(GstGvaMetaConvert *converter, GstBuffer *buffer) {
    GST_DEBUG_CATEGORY_INIT(gst_onvif_converter_debug, "onvifconverter", 0, "ONVIF converter");

    if (!converter) {
        GST_ERROR("Failed convert to json: GvaMetaConvert is null");
        return FALSE;
    }

    if (!buffer) {
        GST_ERROR_OBJECT(converter, "Failed convert to json: GstBuffer is null");
        return FALSE;
    }

    try {
        if (converter->info) {
            json jframe = get_frame_data(converter, buffer);

            // objects section
            json jframe_objects;
            json roi_detection = convert_roi_detection(converter, buffer);
            if (!roi_detection.empty()) {
                jframe_objects = roi_detection;
            }

            if (jframe_objects.empty()) {
                if (!converter->add_empty_detection_results) {
                    GST_DEBUG_OBJECT(converter, "No detections found. Not posting JSON message");
                    return TRUE;
                }
            }

            if (!jframe.is_null()) {
                if (!jframe_objects.empty()) {
                    jframe["Object"] = jframe_objects;
                }
                std::string json_message = jframe.dump(converter->json_indent);
                GVA::VideoFrame video_frame(buffer, converter->info);
                video_frame.add_message(json_message);
                GST_INFO_OBJECT(converter, "JSON message: %s", json_message.c_str());
            }
        }
#ifdef AUDIO
        else {
            return convert_audio_meta_to_json(converter, buffer);
        }
#endif
    } catch (const std::exception &e) {
        GST_ERROR_OBJECT(converter, "%s", Utils::createNestedErrorMsg(e).c_str());
        return FALSE;
    }
    return TRUE;
}
