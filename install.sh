sudo mv lib/lib* /usr/local/lib
sudo mv lib* /usr/local/lib
cd /usr/local/lib
sudo ln -s libactivemq-cpp.so.19.0.5 libactivemq-cpp.so
sudo ln -s libapr-1.so.0.7.0 libapr-1.so
echo "/usr/local/lib" | sudo tee  /etc/ld.so.conf.d/testingharness.conf
sudo ldconfig
