source /opt/intel/openvino_2022/setupvars.sh
source /opt/intel/dlstreamer/gstreamer/setupvars.sh
source ~/dev/dlstreamer/scripts/setup_env.sh

export GST_PLUGIN_PATH=$GST_PLUGIN_PATH:/usr/lib/x86_64-linux-gnu/gstreamer-1.0/
# export OUTPUT=~/dev/dlstreamer/dev_scripts/metaconvert_onvif_out.json

python3 /home/daoyi/dev/video-analytics-use-case-demo/occupancy-analytics/occupancy-analytics.py --pravega-retention-days 0.1 --input-stream camera1 --output-video-stream camera1-infer --recovery-table camera1-rec --pravega-scope examples --keyframe-interval 24 --crossing-line "410, 258, 957, 632"