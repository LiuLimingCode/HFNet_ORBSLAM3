#include "Extractors/HFNetTFModel.h"
using namespace cv;
using namespace std;
using namespace tensorflow;

namespace ORB_SLAM3
{

#ifdef USE_TENSORFLOW

bool HFNetTFModel::Detect(const cv::Mat &image, std::vector<cv::KeyPoint> &vKeypoints, cv::Mat &descriptors,
                int nKeypointsNum, int nRadius)
{
    Tensor tKeypointsNum(DT_INT32, TensorShape());
    Tensor tRadius(DT_INT32, TensorShape());
    tKeypointsNum.scalar<int>()() = nKeypointsNum;
    tRadius.scalar<int>()() = nRadius;

    Tensor tImage(DT_FLOAT, TensorShape({1, image.rows, image.cols, 1}));
    Mat2Tensor(image, &tImage);
    
    vector<Tensor> outputs;
    Status status = session->Run({{"image:0", tImage},{"pred/simple_nms/radius", tRadius},{"pred/top_k_keypoints/k", tKeypointsNum}},
                                 {"keypoints", "local_descriptors", "scores"}, {}, &outputs);
    if (!status.ok()) return false;

    int nResNumber = outputs[0].shape().dim_size(1);

    auto vResKeypoints = outputs[0].tensor<int32, 3>();
    auto vResLocalDes = outputs[1].tensor<float, 3>();
    auto vResScores = outputs[2].tensor<float, 2>();

    vKeypoints.clear();
    descriptors = Mat::zeros(nResNumber, 256, CV_32F);

    KeyPoint kp;
    for(int index = 0; index < nResNumber; index++)
    {
        kp.pt = Point2f(vResKeypoints(2 * index), vResKeypoints(2 * index + 1));
        kp.response = vResScores(index);
        vKeypoints.push_back(kp);
        for (int temp = 0; temp < 256; ++temp)
        {
            descriptors.ptr<float>(index)[temp] = vResLocalDes(256 * index + temp); 
        }
    }
    return true;
}

bool HFNetTFModel::LoadResamplerOp(const std::string &strResamplerDir)
{
    TF_Status *status = TF_NewStatus();
    TF_LoadLibrary(strResamplerDir.c_str(), status);
    if (TF_GetCode(status) != TF_OK) {
        std::cerr << "TF_LoadLibrary() error with code: " << TF_GetCode(status) << std::endl;
        std::cerr << "Failed to load resampler.so in path: " << strResamplerDir << std::endl;
        return false;
    }
    std::cout << "Sucessfully loaded resampler.so" << std::endl;
    return true;
}

bool HFNetTFModel::LoadHFNetTFModel(const std::string &strModelDir)
{
    tensorflow::Status status;
    tensorflow::SessionOptions sessionOptions;
    tensorflow::RunOptions runOptions;
    tensorflow::SavedModelBundle bundle;
    
    status = LoadSavedModel(sessionOptions, runOptions, strModelDir, {tensorflow::kSavedModelTagServe}, &bundle);
    if(!status.ok()){
        std::cerr << "Failed to load HFNet model at path: " << strModelDir <<std::endl;
        return false;
    }

    session = std::move(bundle.session);
    status = session->Create(graph);
    if(!status.ok()){
        std::cerr << "Failed to create graph for HFNet" << std::endl;
        return false;
    }

    std::cout << "Sucessfully loaded HFNet model" << std::endl;
    return true;
}

void HFNetTFModel::Mat2Tensor(const cv::Mat &image, tensorflow::Tensor *tensor)
{
    float *p = tensor->flat<float>().data();
    cv::Mat imagepixel(image.rows, image.cols, CV_32F, p);
    image.convertTo(imagepixel, CV_32F);
}

#endif

}