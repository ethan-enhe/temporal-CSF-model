#include <iostream>
#include <opencv2/opencv.hpp>

#include "opencv2/core.hpp"
#include "opencv2/core/mat.hpp"

using namespace cv;
using namespace std;
bool show = true;

cv::Mat block_proc(cv::Mat I, int tp, pair<int, int> blockSize, pair<int, int> resblockSize,
                   function<cv::Mat(cv::Mat)> func) {
    cv::Mat block, ret(I.size().height / blockSize.first * resblockSize.first,
                       I.size().width / blockSize.second * resblockSize.second, tp);
    // cerr<<ret.size()<<endl;
    for (int y = 0; y < I.rows; y += blockSize.first) {
        for (int x = 0; x < I.cols; x += blockSize.second) {
            cv::Rect roi(x, y, blockSize.second, blockSize.first);
            cv::Rect _roi(x / blockSize.second * resblockSize.second, y / blockSize.first * resblockSize.first,
                          resblockSize.second, resblockSize.first);
            block = I(roi);
            block = func(block);
            block.copyTo(ret(_roi));
        }
    }
    return ret;
}

pair<cv::Mat, cv::Mat> temporalCsfModel(Mat I, Mat I2, int fr, int p) {
    if (I.channels() > 1) {
        cvtColor(I, I, COLOR_BGR2GRAY);
    }
    if (I2.channels() > 1) {
        cvtColor(I2, I2, COLOR_BGR2GRAY);
    }
    cv::Mat I_dct = block_proc(I, CV_32F, {8, 8}, {8, 8}, [&](cv::Mat block) {
        block.convertTo(block, CV_32F);
        cv::Mat block_dct;
        cv::dct(block, block_dct);
        return block_dct;
    });

    if (show) {
        cv::imshow("I_dct", I_dct);
        cv::waitKey(0);
    }

    // Luminance adaptation effect
    cv::Mat F_lum0 = block_proc(I, CV_32F, {8, 8}, {1, 1}, [&](cv::Mat block) {
        cv::Scalar mean = cv::mean(block);
        cv::Mat res(1, 1, CV_32F);
        res.at<float>(0, 0) = mean[0];
        return res;
    });
    if (show) {
        cv::imshow("F_lum0", F_lum0 / 255.0f);
        cv::waitKey(0);
    }

    cv::Mat F_lum = block_proc(F_lum0, CV_32F, {1, 1}, {1, 1}, [&](cv::Mat block) {
        cv::Mat res(1, 1, CV_32F);
        float v = block.at<float>(0, 0);
        if (v <= 60)
            v = (60 - v) / 150 + 1;
        else if (v >= 170)
            v = (v - 170) / 425 + 1;
        else
            v = 1;
        res.at<float>(0, 0) = v;
        return res;
    });
    if (show) {
        cv::imshow("F_lum", F_lum / 2);
        cv::waitKey(0);
    }
    F_lum *= 8;

    // T_basic
    float theta_xy = 2. * atan(1. / (2 * 3 * 768)); // this depend on the resolution
    auto phi = [](int x) {
        if (x == 0)
            return sqrt(1. / 8);
        else
            return sqrt(2. / 8);
    };
    auto omega = [&](int x, int y) {
        // cout<<theta_xy<<endl;
        return sqrt(x * x + y * y) / 16 / theta_xy;
    };
    cv::Mat T_(8, 8, CV_32F);
    float r = 0.6, s = 0.25;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float omega_ij = omega(i, j);
            float phi_ij = asin(2 * omega(i, 0) * omega(j, 0) / omega_ij / omega_ij);
            float cos_phi_ij = cos(phi_ij);
            T_.at<float>(i, j) = 1. / phi(i) / phi(j) * exp(0.18 * omega_ij) / (1.33 + 0.11 * omega_ij) /
                                 (r + (1 - r) * cos_phi_ij * cos_phi_ij);
            cout << omega_ij << endl;
        }
    }
    T_ *= s;
    cv::Mat T_basic = block_proc(F_lum0, CV_32F, {1, 1}, {8, 8}, [&](cv::Mat block) { return T_; });
    if (show) {
        cv::imshow("T", T_basic);
        cv::waitKey(0);
    }

    cv::Mat x, y;
    return {x, y};
}

int main() {
    VideoCapture vid("test.mp4");

    int idx = 1;
    cv::Mat I, I2;
    vid.set(CAP_PROP_POS_FRAMES, idx);
    vid.read(I);
    if (show) {
        cv::imshow("I", I);
        cv::waitKey(0);
    }
    vid.read(I2);

    int fr = vid.get(CAP_PROP_FPS);
    int p = 7;
    auto [T_JND_s, T_JND] = temporalCsfModel(I, I2, fr, p);

    vid.release();
    return 0;
}
