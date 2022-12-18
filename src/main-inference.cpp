#include <Arduino.h>
#include <vector>

#include "model.h"

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Tensorflow
// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
int inference_count = 0;

constexpr int kTensorArenaSize = 1400;
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

// Accel
const int x_pin = 13;
const int y_pin = 14;
const int z_pin = 15;

float x_raw, y_raw, z_raw;
float x_sen, y_sen, z_sen;
float x_bias, y_bias, z_bias;
float x_accel, y_accel, z_accel;

void read_accel();
void scale_accel();
void print_accel();

void setup() {
    Serial.begin(115200);

    /* TF setup */
    tflite::InitializeTarget();

    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    model = tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
    printf(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    return;
    }

    // This pulls in all the operation implementations we need.
    // NOLINTNEXTLINE(runtime-global-variables)
    static tflite::AllOpsResolver resolver;

    static tflite::ErrorReporter *error_reporter;
    static tflite::MicroErrorReporter micro_error;
    error_reporter = &micro_error;

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        printf("AllocateTensors() failed");
        return;
    }

    //   Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

    /* Setup the accelerometer */
    x_sen = 0.3;
    y_sen = 0.3;
    z_sen = 0.3;

    x_bias = 1.5;
    y_bias = 1.5;
    z_bias = 1.5;
}

// From jclay on Github
template <typename T, typename A>
int arg_max(std::vector<T, A> const& vec) {
  return static_cast<int>(std::distance(vec.begin(), max_element(vec.begin(), vec.end())));
}

void loop() {
    // Sample accel and run inference
    read_accel();
    scale_accel();
    // print_accel();
    input->data.f[0] = x_accel;
    input->data.f[1] = y_accel;
    input->data.f[2] = z_accel;
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      printf("Invoke failed on x\n");
      return;
    }
    float v0 = output->data.f[0];
    float v1 = output->data.f[1];
    float v2 = output->data.f[2];
    float v3 = output->data.f[3];
    float v4 = output->data.f[4];
    float v5 = output->data.f[5];
    std::vector<float> v = {v0, v1, v2, v3, v4, v5};
    int pred = arg_max(v);
    printf("%d\n", pred);
    delay(100);
}

void read_accel() {
    x_raw = (float) analogRead(x_pin) / (float)1023;
    y_raw = (float) analogRead(y_pin) / (float)1023;
    z_raw = (float) analogRead(z_pin) / (float)1023; 
}

void scale_accel() {
    x_accel = (x_raw - x_bias) / x_sen;
    y_accel = (y_raw - y_bias) / y_sen;
    z_accel = (z_raw - z_bias) / z_sen;
}
void print_accel(){
    Serial.printf("x: %f y: %f z: %f \n", x_accel, y_accel, z_accel);
}
