# HFNet-SLAM

## An accurate and real-time monocular SLAM system with deep features

More detailed README.md is coming soon ....

HFNet-SLAM is the combination and extension of the well-known [ORB-SLAM3](https://github.com/UZ-SLAMLab/ORB_SLAM3) SLAM framework and a unified CNN model called [HF-Net](https://github.com/ethz-asl/hfnet). It uses the image features from HF-Net to fully replace the hand-crafted ORB features and the BoW method in the ORB-SLAM3 system. This novelty results in better performance in tracking and loop closure, boosting the accuracy of the entire HFNet-SLAM.

## HFNet-SLAM vs ORB-SLAM3

**Better Tracking**:

<img src="https://user-images.githubusercontent.com/52725165/197087949-21196670-335e-4ea9-ac12-f226521da691.png" width="600" title="Better Tracking">

**Better Loop Closure**:

<img src="https://user-images.githubusercontent.com/52725165/197088191-1d01fe8a-02ef-4002-8eeb-3c312ef48eb4.png" width="600" title="Better Loop Closure">

**Comparative Runtime Performance**:

<img src="https://user-images.githubusercontent.com/52725165/197371705-e437adc0-ed47-4bb7-a3db-ff0a091b2568.png" title="Comparative Runtime Performance">

HFNet-SLAM has more effective mapping and loop detection threads compared with ORB-SLAM3, but it needs an extra 10 ms and GPU support in the tracking tread because the inference of the HF-Net model uses float64 precision. There is a great potential improvement by using quantization and half-precision technologies to increase the runtime performance.

## Prerequisites

### OpenCV

We use OpenCV to manipulate images and features. Dowload and install instructions can be found at: http://opencv.org. Required at leat 4.4.

### TensorFlow C++ API

We use TensorFlow C++, CUDA, and cuDNN for model inference. The corresponding version of these libraries should be chosen wisely according to the devices. The following configurations has been tested:

| Name   | Version  |
| --------   | :-------:  |
| Ubuntu | 20.04     |
| GPU     | RTX 2070 Max-Q   |
| NVIDIA Driver   |  510.47  |
| CUDA tool  |  11.6.2 |
| cuDNN | 8.4.1.50 |
| TensorFlow | 1.15, 2.9 |

The download and install instructions of CUDA can be found at: https://developer.nvidia.com/cuda-toolkit.

The instructions of cuDNN can be found at: https://docs.nvidia.com/deeplearning/cudnn/install-guide/index.html.

An easy method for building TensorFlow C++ API can be found at: https://github.com/FloopCZ/tensorflow_cc.

### Download the HF-Net Model

Download the converted model from [here](https://drive.google.com/file/d/1vBmC5t3NDIBULh8xeM2A_Qs2aDlaMAOz/view?usp=share_link).

More details about the process of converting can be found in the [HF-Net Model Converting](hfnet/README.md) document.

## Building HFNet-SLAM library and examples

```
bash build.sh
```

## Evaluation on EuRoC dataset

https://user-images.githubusercontent.com/52725165/197089468-99c7ebf2-18c7-45da-a62e-b69691c3d248.mp4

Evaluate a single sequence with the pure monocular configuration:

```
sequenceName='MH01'
./Examples/Monocular/mono_euroc ./Examples/Monocular/EuRoC.yaml "$pathEvaluation"/"$sequenceName"_MONO/ "$pathDataset"/"$sequenceName" ./Examples/Monocular/EuRoC_TimeStamps/"$sequenceName".txt
python3 ./evaluation/evaluate_ate_scale.py ./evaluation/Ground_truth/EuRoC_left_cam/"$sequenceName"_GT.txt "$pathEvaluation"/"$sequenceName"_MONO/trajectory.txt --verbose --save_path "$pathEvaluation"/"$sequenceName"_MONO/
```

Evaluate a single sequence with the monocular-inertial configuration:

```
sequenceName='MH01'
./Examples/Monocular-Inertial/mono_inertial_euroc ./Examples/Monocular-Inertial/EuRoC.yaml "$pathEvaluation"/"$sequenceName"_MONO_IN/ "$pathDataset"/"$sequenceName" ./Examples/Monocular-Inertial/EuRoC_TimeStamps/"$sequenceName".txt
python3 ./evaluation/evaluate_ate_scale.py "$pathDataset"/"$sequenceName"/mav0/state_groundtruth_estimate0/data.csv "$pathEvaluation"/"$sequenceName"_MONO_IN/trajectory.txt --verbose --save_path "$pathEvaluation"/"$sequenceName"_MONO_IN/
```

Evaluate the whole dataset:

```
bash Examples/eval_euroc.sh
```

## Evaluation on TUM-VI dataset

https://user-images.githubusercontent.com/52725165/197089404-e2c96c00-cfb0-4f73-983a-94476a01c009.mp4

Evaluate a single sequence with the monocular-inertial configuration:

In 'outdoors' sequences, Use './Examples/Monocular-Inertial/TUM-VI_far.yaml' configuration file instead.

```
sequenceName='dataset-corridor1_512'
./Examples/Monocular-Inertial/mono_inertial_tum_vi ./Examples/Monocular-Inertial/TUM-VI.yaml "$pathEvaluation"/"$sequenceName"/ "$pathDataset"/"$sequenceName"_16/mav0/cam0/data ./Examples/Monocular-Inertial/TUM_TimeStamps/"$sequenceName".txt ./Examples/Monocular-Inertial/TUM_IMU/"$sequenceName".txt
python3 ./evaluation/evaluate_ate_scale.py "$pathDataset"/"$sequenceName"_16/mav0/mocap0/data.csv "$pathEvaluation"/"$sequenceName"/trajectory.txt --verbose --save_path "$pathEvaluation"/"$sequenceName"/
```

Evaluate the whole dataset:

```
bash Examples/eval_tum_vi.sh
```

