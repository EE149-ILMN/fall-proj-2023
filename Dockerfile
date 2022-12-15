# syntax=docker/dockerfile:1
FROM ubuntu:latest

# Add jupyter notebook
FROM jupyter/tensorflow-notebook
ENV CONDA_DIR=/opt/conda SHELL=/bin/bash NB_USER=jovyan NB_UID=1000 NB_GID=100 LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8 LANGUAGE=en_US.UTF-8
ENV PATH=/opt/conda/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin HOME=/home/jovyan

USER root

# RUN apt-get update && apt install -y apt-transport-https curl gnupg
# RUN curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor >bazel-archive-keyring.gpg
# RUN mv bazel-archive-keyring.gpg /usr/share/keyrings
# RUN echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
# RUN apt-get update && apt-get install -y bazel
# RUN git clone https://github.com/tensorflow/tflite-micro.git
# RUN cd tflite-micro/tensorflow
# RUN bazel build --config=elinux_aarch64 -c opt //tflite-micro/tensorflow/lite:libtensorflowlite.so

# FROM tensorflow/tensorflow:devel-gpu
# RUN bazel build --config=opt --config=cuda //tensorflow/tools/pip_package:build_pip_package
# RUN ./bazel-bin/tensorflow/tools/pip_package/build_pip_package /mnt
# RUN chown $HOST_PERMS /mnt/tensorflow-version-tags.whl

# RUN apt-get update && apt-get install -y cmake
RUN echo $pwd
RUN git clone https://github.com/tensorflow/tflite-micro.git
WORKDIR "/home/jovyan/tflite-micro"
RUN make -f tensorflow/lite/micro/tools/make/Makefile generate_projects

# RUN mkdir tflite_build && cd tflite_build
# RUN cmake ../tflite-micro
# RUN bazel build --config=elinux_aarch64 -c opt //third_party/flatbuffers
# RUN bazel build --config=elinux_aarch64 -c opt //tensorflow/lite:libtensorflowlite.so

WORKDIR "/home/jovyan"
RUN apt-get update && apt-get install -y python3 python3-pip xxd
RUN pip3 install ipython
RUN pip3 install pyserial
RUN pip3 install numpy pandas scikit-learn matplotlib



