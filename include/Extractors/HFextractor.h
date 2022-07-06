#ifndef HFNETEXTRACTOR_H
#define HFNETEXTRACTOR_H

#include <vector>
#include <list>
#include <opencv2/opencv.hpp>
#include "Extractors/BaseModel.h"

namespace ORB_SLAM3
{

class BaseModel;

class HFextractor
{
public:

    HFextractor(int nfeatures, int nNMSRadius, float threshold, BaseModel* pModels);

    HFextractor(int nfeatures, int nNMSRadius, float threshold,
                float scaleFactor, int nlevels, const std::vector<BaseModel*>& vpModels);

    ~HFextractor(){}

    // Compute the features and descriptors on an image.
    int operator()(const cv::Mat &_image, std::vector<cv::KeyPoint>& _keypoints,
                   cv::Mat &_localDescriptors, cv::Mat &_globalDescriptors);

    int inline GetLevels(void) {
        return nlevels;}

    float inline GetScaleFactor(void) {
        return scaleFactor;}

    std::vector<float> inline GetScaleFactors(void) {
        return mvScaleFactor;
    }

    std::vector<float> inline GetInverseScaleFactors(void) {
        return mvInvScaleFactor;
    }

    std::vector<float> inline GetScaleSigmaSquares(void) {
        return mvLevelSigma2;
    }

    std::vector<float> inline GetInverseScaleSigmaSquares(void) {
        return mvInvLevelSigma2;
    }

    std::vector<cv::Mat> mvImagePyramid;

public:

    double scaleFactor;
    int nlevels;
    bool bUseOctTree;

    std::vector<float> mvScaleFactor;
    std::vector<float> mvInvScaleFactor;    
    std::vector<float> mvLevelSigma2;
    std::vector<float> mvInvLevelSigma2;

    int nfeatures;
    int nNMSRadius;
    float threshold;
    std::vector<BaseModel*> mvpModels;

    std::vector<int> mnFeaturesPerLevel;

    std::vector<int> umax;

    void ComputePyramid(const cv::Mat &image);
};

} //namespace ORB_SLAM

#endif
