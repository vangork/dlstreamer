source /opt/intel/openvino_2022/setupvars.sh
source /opt/intel/dlstreamer/gstreamer/setupvars.sh
source ~/dev/dlstreamer/scripts/setup_env.sh

export DETECTION_MODEL="${MODELS_PATH}/intel/person-vehicle-bike-detection-2004/FP32/person-vehicle-bike-detection-2004.xml"
export DETECTION_MODEL_PROC=/opt/intel/dlstreamer/samples/gstreamer/model_proc/intel/person-vehicle-bike-detection-2004.json
export VEHICLE_CLASSIFICATION_MODEL="${MODELS_PATH}/intel/vehicle-attributes-recognition-barrier-0039/FP32/vehicle-attributes-recognition-barrier-0039.xml"
export VEHICLE_CLASSIFICATION_MODEL_PROC=/opt/intel/dlstreamer/samples/gstreamer/model_proc/intel/vehicle-attributes-recognition-barrier-0039.json
export VIDEO_EXAMPLE=~/dev/person-bicycle-car-detection.mp4
export OUTPUT=~/dev/dlstreamer/dev_scripts/metaconvert_onvif_out.json


gst-launch-1.0 \
filesrc location=${VIDEO_EXAMPLE} ! decodebin ! \
gvadetect model=${DETECTION_MODEL} model_proc=${DETECTION_MODEL_PROC} device=CPU ! queue ! \
gvaclassify model=${VEHICLE_CLASSIFICATION_MODEL} model-proc=${VEHICLE_CLASSIFICATION_MODEL_PROC} device=CPU object-class=vehicle ! queue ! \
gvametaconvert format=onvif-json ! \
gvametapublish method=file file-format=json file-path=${OUTPUT} ! \
fakesink sync=false