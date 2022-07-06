/**
 * To test the tensorflow api, and the base function of HFNet
 * 
 * Result:
session->Run() output size: 4
outputs[0].shape(): [1,1000,2]
outputs[1].shape(): [1,1000,256]
outputs[2].shape(): [1,1000]
outputs[3].shape(): [1,4096]
 *
 */
#include <chrono>
#include <fstream>
#include <dirent.h>
#include <random>

#include "Settings.h"
#include "Extractors/HFNetTFModel.h"
#include "Extractors/HFextractor.h"
#include "Examples/Utility/utility_common.h"

using namespace cv;
using namespace std;
using namespace ORB_SLAM3;
using namespace tensorflow;

Settings *settings;
HFNetTFModel *pModel;
TicToc timerDetect;
TicToc timerRun;

void Mat2Tensor(const cv::Mat &image, tensorflow::Tensor *tensor)
{
    float *p = tensor->flat<float>().data();
    cv::Mat imagepixel(image.rows, image.cols, CV_32F, p);
    image.convertTo(imagepixel, CV_32F);
}

bool DetectOnlyLocal(const cv::Mat &image, std::vector<cv::KeyPoint> &vKeypoints, cv::Mat &localDescriptors,
                     int nKeypointsNum, int nRadius, float threshold)
{
    Tensor tKeypointsNum(DT_INT32, TensorShape());
    Tensor tRadius(DT_INT32, TensorShape());
    Tensor tThreshold(DT_FLOAT, TensorShape());
    tKeypointsNum.scalar<int>()() = nKeypointsNum;
    tRadius.scalar<int>()() = nRadius;
    tThreshold.scalar<float>()() = threshold;

    Tensor tImage(DT_FLOAT, TensorShape({1, image.rows, image.cols, 1}));
    Mat2Tensor(image, &tImage);
    
    vector<Tensor> outputs;
    timerRun.Tic();
    Status status = pModel->mSession->Run({{"image:0", tImage},
                                           {"pred/simple_nms/radius", tRadius},
                                           {"pred/top_k_keypoints/k", tKeypointsNum},
                                           {"pred/keypoint_extraction/GreaterEqual/y", tThreshold}},
                                          {"keypoints", "local_descriptors", "scores"}, {}, &outputs);
    timerRun.Toc();
    if (!status.ok()) return false;

    int nResNumber = outputs[0].shape().dim_size(1);

    auto vResKeypoints = outputs[0].tensor<int32, 3>();
    auto vResLocalDes = outputs[1].tensor<float, 3>();
    auto vResScores = outputs[2].tensor<float, 2>();

    vKeypoints.clear();
    vKeypoints.reserve(nResNumber);
    localDescriptors = cv::Mat(nResNumber, 256, CV_32F);
    KeyPoint kp;
    kp.angle = 0;
    kp.octave = 0;
    for(int index = 0; index < nResNumber; index++)
    {
        kp.pt = Point2f(vResKeypoints(2 * index), vResKeypoints(2 * index + 1));
        kp.response = vResScores(index);
        vKeypoints.emplace_back(kp);
        for (int temp = 0; temp < 256; ++temp)
        {
            localDescriptors.ptr<float>(index)[temp] = vResLocalDes(256 * index + temp); 
        }
    }
    return true;
}

bool DetectFull(const cv::Mat &image, std::vector<cv::KeyPoint> &vKeypoints, cv::Mat &localDescriptors, cv::Mat &globalDescriptors,
                int nKeypointsNum, int nRadius, float threshold)
{
    Tensor tKeypointsNum(DT_INT32, TensorShape());
    Tensor tRadius(DT_INT32, TensorShape());
    Tensor tThreshold(DT_FLOAT, TensorShape());
    tKeypointsNum.scalar<int>()() = nKeypointsNum;
    tRadius.scalar<int>()() = nRadius;
    tThreshold.scalar<float>()() = threshold;

    Tensor tImage(DT_FLOAT, TensorShape({1, image.rows, image.cols, 1}));
    Mat2Tensor(image, &tImage);
    
    vector<Tensor> outputs;
    timerRun.Tic();
    Status status = pModel->mSession->Run({{"image:0", tImage},
                                           {"pred/simple_nms/radius", tRadius},
                                           {"pred/top_k_keypoints/k", tKeypointsNum},
                                           {"pred/keypoint_extraction/GreaterEqual/y", tThreshold}},
                                          {"keypoints", "local_descriptors", "scores", "global_descriptor"}, {}, &outputs);
    timerRun.Toc();
    if (!status.ok()) return false;

    int nResNumber = outputs[0].shape().dim_size(1);

    auto vResKeypoints = outputs[0].tensor<int32, 3>();
    auto vResLocalDes = outputs[1].tensor<float, 3>();
    auto vResScores = outputs[2].tensor<float, 2>();
    auto vResGlobalDes = outputs[3].tensor<float, 2>();

    // cout << "session->Run() output size: " << outputs.size() << endl;
    // cout << "outputs[0].shape(): " << outputs[0].shape() << endl;
    // cout << "outputs[1].shape(): " << outputs[1].shape() << endl;
    // cout << "outputs[2].shape(): " << outputs[2].shape() << endl;
    // cout << "outputs[3].shape(): " << outputs[3].shape() << endl;

    vKeypoints.clear();
    vKeypoints.reserve(nResNumber);
    localDescriptors = cv::Mat(nResNumber, 256, CV_32F);
    KeyPoint kp;
    kp.angle = 0;
    kp.octave = 0;
    for(int index = 0; index < nResNumber; index++)
    {
        kp.pt = Point2f(vResKeypoints(2 * index), vResKeypoints(2 * index + 1));
        kp.response = vResScores(index);
        vKeypoints.emplace_back(kp);
        for (int temp = 0; temp < 256; ++temp)
        {
            localDescriptors.ptr<float>(index)[temp] = vResLocalDes(256 * index + temp); 
        }
    }
    globalDescriptors = cv::Mat(4096, 1, CV_32F);
    for (int temp = 0; temp < 4096; ++temp)
    {
        globalDescriptors.ptr<float>(0)[temp] = vResGlobalDes(temp);
    }
    return true;
}

// const string strDatasetPath("/media/llm/Datasets/EuRoC/MH_04_difficult/mav0/cam0/data/");
// const string strSettingsPath("Examples/Monocular-Inertial/EuRoC.yaml");
// const int dbStart = 420;
// const int dbEnd = 50;

const string strDatasetPath("/media/llm/Datasets/TUM-VI/dataset-corridor4_512_16/mav0/cam0/data/");
const string strSettingsPath("Examples/Monocular-Inertial/TUM-VI.yaml");
const int dbStart = 50;
const int dbEnd = 50;

int main(int argc, char* argv[])
{
    settings = new Settings(strSettingsPath, 0);
    const string strResamplerPath = "/home/llm/src/tensorflow-1.15.5/bazel-bin/tensorflow/contrib/resampler/python/ops/_resampler_ops.so";
    const string strModelPath = "/home/llm/ROS/HFNet_ORBSLAM3_v2/model/hfnet_tf/";
    pModel = new HFNetTFModel(strResamplerPath, strModelPath);

    vector<string> files = GetPngFiles(strDatasetPath); // get all image files
    
    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned int> distribution(dbStart, files.size() - dbEnd);

    cv::Mat image;
    vector<KeyPoint> vKeypoints;
    cv::Mat localDescriptors, globalDescriptors;
    
    // randomly detect an image and show the results
    char command = ' ';
    float threshold = 0;
    int nNMSRadius = 4;
    int select = 0;
    while(1)
    {
        if (command == 'q') break;
        else if (command == 's') select = std::max(select - 1, 0);
        else if (command == 'w') select += 1;
        else if (command == 'a') threshold = std::max(threshold - 0.01, 0.0);
        else if (command == 'd') threshold += 0.01;
        else if (command == 'z') nNMSRadius = std::max(nNMSRadius - 1, 0);
        else if (command == 'c') nNMSRadius += 1;
        else select = distribution(generator);
        cout << "command: " << command << endl;
        cout << "select: " << select << endl;
        cout << "threshold: " << threshold << endl;
        cout << "nNMSRadius: " << nNMSRadius << endl;

        image = imread(strDatasetPath + files[select], IMREAD_GRAYSCALE);
        if (settings->needToResize())
            cv::resize(image, image, settings->newImSize());
        
        DetectFull(image, vKeypoints, localDescriptors, globalDescriptors, 1000, nNMSRadius, threshold);
        cout << "Get features number: " << vKeypoints.size() << endl;
        
        ShowKeypoints("press 'q' to exit", image, vKeypoints);
        cout << endl;
        command = cv::waitKey();
    }
    cv::destroyAllWindows();

    // detect full dataset
    {
        image = imread(strDatasetPath + files[0], IMREAD_GRAYSCALE);
        if (settings->needToResize())
            cv::resize(image, image, settings->newImSize());
        DetectOnlyLocal(image, vKeypoints, localDescriptors, settings->nFeatures(), settings->nNMSRadius(), settings->threshold());
        
        timerDetect.clearBuff();
        timerRun.clearBuff();
        for (const string& file : files)
        {
            image = imread(strDatasetPath + file, IMREAD_GRAYSCALE);
            if (settings->needToResize())
                cv::resize(image, image, settings->newImSize());
            timerDetect.Tic();
            DetectOnlyLocal(image, vKeypoints, localDescriptors, settings->nFeatures(), settings->nNMSRadius(), settings->threshold());
            timerDetect.Toc();
        }
        cout << "Only detect the local keypoints: " << endl
             << "run cost time: " << timerRun.aveCost() << " milliseconds" << endl
             << "detect cost time: " << timerDetect.aveCost() << " milliseconds" << endl;
    }
    {
        image = imread(strDatasetPath + files[0], IMREAD_GRAYSCALE);
        if (settings->needToResize())
            cv::resize(image, image, settings->newImSize());
        DetectFull(image, vKeypoints, localDescriptors, globalDescriptors, settings->nFeatures(), settings->nNMSRadius(), settings->threshold());
        
        timerDetect.clearBuff();
        timerRun.clearBuff();
        for (const string& file : files)
        {
            image = imread(strDatasetPath + file, IMREAD_GRAYSCALE);
            if (settings->needToResize())
                cv::resize(image, image, settings->newImSize());
            timerDetect.Tic();
            DetectFull(image, vKeypoints, localDescriptors, globalDescriptors, settings->nFeatures(), settings->nNMSRadius(), settings->threshold());
            timerDetect.Toc();
        }
        cout << "Detect the full features: " << endl
             << "run cost time: " << timerRun.aveCost() << " milliseconds" << endl
             << "detect cost time: " << timerDetect.aveCost() << " milliseconds" << endl;
    }
    {
        HFextractor extractor = HFextractor(settings->nFeatures(),settings->nNMSRadius(),settings->threshold(),1.0,1,{pModel});
        image = imread(strDatasetPath + files[0], IMREAD_GRAYSCALE);
        if (settings->needToResize())
            cv::resize(image, image, settings->newImSize());
        extractor(image, vKeypoints, localDescriptors, globalDescriptors);

        timerDetect.clearBuff();
        timerRun.clearBuff();
        vKeypoints.clear();
        for (const string& file : files)
        {
            image = imread(strDatasetPath + file, IMREAD_GRAYSCALE);
            if (settings->needToResize())
                cv::resize(image, image, settings->newImSize());
            timerDetect.Tic();
            extractor(image, vKeypoints, localDescriptors, globalDescriptors);
            timerDetect.Toc();
        }
        cout << "Detect the full features with HFextractor: " << endl
             << "detect cost time: " << timerDetect.aveCost() << " milliseconds" << endl;
    }

    getchar();

    return 0;
}