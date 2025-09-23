// PeopleCounter.cpp
#include <jni.h>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace dnn;
using namespace std;

const float CONFIDENCE_THRESHOLD = 0.3;
const float NMS_THRESHOLD = 0.4;
const int PERSON_CLASS_ID = 0;

vector<string> loadClassNames(const string& filename) {
    vector<string> classNames;
    ifstream ifs(filename.c_str());
    string line;
    while (getline(ifs, line)) {
        classNames.push_back(line);
    }
    return classNames;
}

vector<String> getOutputLayerNames(const Net& net) {
    static vector<String> names;
    if (names.empty()) {
        vector<int> outLayers = net.getUnconnectedOutLayers();
        vector<String> layersNames = net.getLayerNames();
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i)
            names[i] = layersNames[outLayers[i] - 1];
    }
    return names;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_example_modbus_PeopleCounter_detectPeople(JNIEnv* env, jobject obj, jstring jImagePath) {
    // Rest of your code remains unchanged
    const char* imagePath = env->GetStringUTFChars(jImagePath, nullptr);

    string modelConfiguration = "models/yolov4-tiny.cfg";
    string modelWeights = "models/yolov4-tiny.weights";
    string cocoNames = "models/coco.names";

    Net net = readNetFromDarknet(modelConfiguration, modelWeights);
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);

    vector<string> classNames = loadClassNames(cocoNames);

    Mat image = imread(imagePath);
    if (image.empty()) {
        cerr << "Error loading image: " << imagePath << endl;
        env->ReleaseStringUTFChars(jImagePath, imagePath);
        return -1;
    }

    Mat blob;
    blobFromImage(image, blob, 1/255.0, Size(416, 416), Scalar(), true, false);
    net.setInput(blob);

    vector<Mat> outputs;
    net.forward(outputs, getOutputLayerNames(net));

    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;

    for (const Mat& output : outputs) {
        for (int i = 0; i < output.rows; ++i) {
            const float* data = output.ptr<float>(i);
            float confidence = data[4];
            if (confidence >= CONFIDENCE_THRESHOLD) {
                Mat scores = output.row(i).colRange(5, output.cols);
                Point classIdPoint;
                double maxClassScore;
                minMaxLoc(scores, 0, &maxClassScore, 0, &classIdPoint);
                if (maxClassScore > CONFIDENCE_THRESHOLD) {
                    int centerX = static_cast<int>(data[0] * image.cols);
                    int centerY = static_cast<int>(data[1] * image.rows);
                    int width = static_cast<int>(data[2] * image.cols);
                    int height = static_cast<int>(data[3] * image.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back(static_cast<float>(maxClassScore));
                    boxes.emplace_back(left, top, width, height);
                }
            }
        }
    }

    vector<int> indices;
    NMSBoxes(boxes, confidences, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);

    int peopleCount = 0;

    for (int idx : indices) {
        if (classIds[idx] == PERSON_CLASS_ID) {
            peopleCount++;
            Rect box = boxes[idx];
            rectangle(image, box, Scalar(0, 255, 0), 2);
            putText(image, "Person", Point(box.x, box.y - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
        }
    }

    // imwrite("yolo_output.jpg", image);

    env->ReleaseStringUTFChars(jImagePath, imagePath);

    return peopleCount;
}