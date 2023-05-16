source /opt/intel/openvino_2022/setupvars.sh
source /opt/intel/dlstreamer/gstreamer/setupvars.sh
source ~/dev/dlstreamer/scripts/setup_env.sh

export GST_PLUGIN_PATH=$GST_PLUGIN_PATH:/usr/lib/x86_64-linux-gnu/gstreamer-1.0/
export ROI="[{\"x\":600,\"y\":180,\"w\":600,\"h\":500}]"
export INFLUXDB_DEVICE=cam1
# export OUTPUT=~/dev/dlstreamer/dev_scripts/metaconvert_onvif_out.json

python3 ~/dev/video-analytics-use-case-demo/human-detection/human-detection.py --pravega-controller-uri tcp://127.0.0.1:9090 --pravega-scope examples --pravega-retention-policy-type days --pravega-retention-days 0.1 --input-stream camera1 --output-video-stream camera1-infer --recovery-table camera1-rec