cd ~/dev/dlstreamer
source /opt/intel/openvino_2022/setupvars.sh
source /opt/intel/dlstreamer/gstreamer/setupvars.sh
mkdir build
cd build
cmake -DPRAVEGA_CLIENT_INCLUDE_DIR=/usr/lib ..
make -j
sudo cp ./intel64/Release/lib/libgvametapublishpravega.so /opt/intel/dlstreamer/gstreamer/lib/gstreamer-1.0
sudo cp ./intel64/Release/lib/libgvametapublish.so /opt/intel/dlstreamer/gstreamer/lib/gstreamer-1.0
